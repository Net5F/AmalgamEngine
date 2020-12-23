#include "NpcMovementSystem.h"
#include "MovementHelpers.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "ClientNetworkDefs.h"
#include "EntityUpdate.h"
#include "Name.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Input.h"
#include "Sprite.h"
#include "PlayerState.h"
#include "Log.h"
#include "Ignore.h"
#include "entt/entity/registry.hpp"
#include <memory>
#include <string>

namespace AM
{
namespace Client
{
NpcMovementSystem::NpcMovementSystem(Game& inGame, World& inWorld,
                                     Network& inNetwork)
: lastReceivedTick(0)
, lastProcessedTick(0)
, tickReplicationOffset(-2 * INITIAL_TICK_OFFSET)
, game(inGame)
, world(inWorld)
, network(inNetwork)
{
    // Init the groups that we'll be using.
    auto group = world.registry.group<Input, Position, PreviousPosition, Movement>(
        entt::exclude<PlayerState>);
    ignore(group);
}

void NpcMovementSystem::updateNpcs()
{
    if (RUN_OFFLINE) {
        // No need to process NPCs if we're running offline.
        return;
    }

    // Receive any updates from the server, update pastTickOffset.
    receiveEntityUpdates();

    // We want to process updates until we've either processed the desired, or
    // run out of data.
    Uint32 desiredTick = game.getCurrentTick() + tickReplicationOffset;

    /* While we have data to use, apply updates for all unprocessed ticks
       including the desired tick. */
    bool updated = false;
    while ((lastProcessedTick <= desiredTick)
           && (stateUpdateQueue.size() > 0)) {
        updated = true;
        // Move all NPCs as if their inputs didn't change.
        moveAllNpcs();

        // Check that the processed tick is progressing incrementally.
        NpcStateUpdate& stateUpdate = stateUpdateQueue.front();
        if (stateUpdate.tickNum != (lastProcessedTick + 1)) {
            LOG_ERROR("Processing NPC movement out of order. "
                      "stateUpdate.tickNum: %u, "
                      "lastProcessedTick: %u",
                      stateUpdate.tickNum, lastProcessedTick);
        }

        // If the update contained new data, apply it.
        if (stateUpdate.dataChanged) {
            applyUpdateMessage(stateUpdate.entityUpdate);
        }

        /* Prepare for the next iteration. */
        lastProcessedTick++;
        stateUpdateQueue.pop();
    }

    // If we're initialized and needed to process a tick but didn't have data,
    // log it.
    if (!updated && (lastReceivedTick != 0)
        && (lastProcessedTick <= desiredTick)) {
        LOG_INFO("Tick passed with no npc update. last: %u, desired: %u, "
                 "queueSize: %u, offset: %d",
                 lastProcessedTick, desiredTick, stateUpdateQueue.size(),
                 tickReplicationOffset);
    }
}

void NpcMovementSystem::applyTickAdjustment(int adjustment)
{
    // If the server tells us to add 2 ticks of latency, it's saying that it
    // needs 2 more ticks worth of time for our messages to arrive in time.
    // NPC replication should now be delayed by an additional 4 ticks (2 for
    // each direction).
    // Thus, we negate and double the adjustment before applying.
    tickReplicationOffset += (-2 * adjustment);

    if (tickReplicationOffset >= 0) {
        LOG_ERROR("Adjusted tickReplicationOffset too far into the future. "
                  "offset: %u",
                  tickReplicationOffset);
    }
}

void NpcMovementSystem::receiveEntityUpdates()
{
    /* Process any NPC update messages from the Network. */
    NpcReceiveResult receiveResult = network.receiveNpcUpdate();
    while (receiveResult.result == NetworkResult::Success) {
        NpcUpdateMessage& npcUpdateMessage = receiveResult.message;

        // Handle the message appropriately.
        switch (npcUpdateMessage.updateType) {
            case NpcUpdateType::ExplicitConfirmation:
                // If we've been initialized, process the confirmation.
                if (lastReceivedTick != 0) {
                    handleExplicitConfirmation();
                }
                break;
            case NpcUpdateType::ImplicitConfirmation:
                // If we've been initialized, process the confirmation.
                if (lastReceivedTick != 0) {
                    handleImplicitConfirmation(npcUpdateMessage.tickNum);
                }
                break;
            case NpcUpdateType::Update:
                handleUpdate(npcUpdateMessage.message);
                break;
        }

        receiveResult = network.receiveNpcUpdate();
    }
}

void NpcMovementSystem::handleExplicitConfirmation()
{
    lastReceivedTick++;
    stateUpdateQueue.push({lastReceivedTick, false, nullptr});
}

void NpcMovementSystem::handleImplicitConfirmation(Uint32 confirmedTick)
{
    // If there's a gap > 1 between the latest received tick and the confirmed
    // tick, we know that no changes happened on the in-between ticks and can
    // push confirmations for them.
    unsigned int implicitConfirmations = confirmedTick - lastReceivedTick;
    for (unsigned int i = 1; i <= implicitConfirmations; ++i) {
        stateUpdateQueue.push({(lastReceivedTick + i), false, nullptr});
    }

    lastReceivedTick = confirmedTick;
}

void NpcMovementSystem::handleUpdate(
    const std::shared_ptr<const EntityUpdate>& entityUpdate)
{
    Uint32 newReceivedTick = entityUpdate->tickNum;

    if (lastReceivedTick != 0) {
        // The update message implicitly confirms all ticks since our last
        // received.
        handleImplicitConfirmation(newReceivedTick - 1);
    }
    else {
        // This is our first received update.
        // Init the last processed tick so things look incrementally increasing.
        lastProcessedTick = newReceivedTick - 1;
    }

    // Push the update into the buffer.
    stateUpdateQueue.push({newReceivedTick, true, entityUpdate});

    lastReceivedTick = newReceivedTick;
}

void NpcMovementSystem::moveAllNpcs()
{
    // Move all NPCs that have an input, position, and movement component.
    auto group = world.registry.group<Input, Position, PreviousPosition, Movement>(
        entt::exclude<PlayerState>);
    for (entt::entity entity : group) {
        Input& input = group.get<Input>(entity);
        Position& position = group.get<Position>(entity);
        PreviousPosition& previousPos = group.get<PreviousPosition>(entity);
        Movement& movement = group.get<Movement>(entity);

        // Save their old position.
        previousPos.x = position.x;
        previousPos.y = position.y;
        previousPos.z = position.z;

        // Process their movement.
        MovementHelpers::moveEntity(position, movement, input.inputStates,
            GAME_TICK_TIMESTEP_S);
    }
}

void NpcMovementSystem::applyUpdateMessage(
    const std::shared_ptr<const EntityUpdate>& entityUpdate)
{
    // Use the data in the message to correct any NPCs that changed inputs.
    const std::vector<EntityState>& entities = entityUpdate->entityStates;
    for (auto entityIt = entities.begin(); entityIt != entities.end();
         ++entityIt) {
        // Skip the player (not an NPC).
        entt::entity entity = entityIt->entity;
        if (entity == world.playerEntity) {
            continue;
        }

        // If the entity doesn't exist, create it.
        entt::registry& registry = world.registry;
        if (!(registry.valid(entity))) {
            LOG_INFO("New entity added. ID: %u", entity);
            entt::entity newEntity = registry.create(entity);
            if (entity != newEntity) {
                LOG_ERROR(
                    "Created entity doesn't match received entity. Created: %u, received: %u",
                    newEntity, entity);
            }

            // Set the name.
            registry.emplace<Name>(entity,
                std::to_string(static_cast<Uint32>(registry.version(entity))));

            // TODO: Get the sprite info from the server.
            // Get the same texture as the player.
            Sprite& playerSprite = registry.get<Sprite>(world.playerEntity);
            registry.emplace<Sprite>(entity, playerSprite);

            // Init their old position so they don't lerp in from elsewhere.
            const Position& receivedPos = entityIt->position;
            registry.emplace<PreviousPosition>(entity, receivedPos.x, receivedPos.y,
                receivedPos.z);

            // Create the rest of their components, to be set for real below.
            registry.emplace<Input>(entity);
            registry.emplace<Position>(entity);
            registry.emplace<Movement>(entity);
        }

        // Update their inputs.
        registry.patch<Input>(entity, [entityIt](Input& input) {input = entityIt->input;});

        // Update their position.
        registry.patch<Position>(entity,
            [entityIt](Position& position) {position = entityIt->position;});

        // Update their movements.
        registry.patch<Movement>(entity,
            [entityIt](Movement& movement) {movement = entityIt->movement;});
    }
}

} // namespace Client
} // namespace AM
