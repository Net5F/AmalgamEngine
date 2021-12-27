#include "NpcMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "EntityUpdate.h"
#include "Name.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "Input.h"
#include "BoundingBox.h"
#include "Sprite.h"
#include "InputHistory.h"
#include "ClientNetworkDefs.h"
#include "Transforms.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Log.h"
#include "Ignore.h"
#include "entt/entity/registry.hpp"
#include <memory>
#include <string>

namespace AM
{
namespace Client
{
NpcMovementSystem::NpcMovementSystem(Simulation& inSim, World& inWorld,
                                     EventDispatcher& inNetworkEventDispatcher,
                                     Network& inNetwork, SpriteData& inSpriteData)
: sim{inSim}
, world{inWorld}
, network{inNetwork}
, npcUpdateQueue{inNetworkEventDispatcher}
, spriteData{inSpriteData}
, lastReceivedTick(0)
, lastProcessedTick(0)
, tickReplicationOffset(Config::INITIAL_REPLICATION_OFFSET)
{
    // Init the groups that we'll be using.
    auto group
        = world.registry.group<Input, Position, PreviousPosition, Movement, BoundingBox, Sprite>(
            entt::exclude<InputHistory>);
    ignore(group);
}

void NpcMovementSystem::updateNpcs()
{
    if (Config::RUN_OFFLINE) {
        // No need to process NPCs if we're running offline.
        return;
    }

    // Receive any updates from the server, update lastReceivedTick.
    receiveEntityUpdates();

    // We want to process updates until we've either processed the desired
    // tick, or run out of data.
    Uint32 desiredTick{sim.getCurrentTick() + tickReplicationOffset};

    /* While we have authoritative data to use, apply updates for all
       unprocessed ticks including the desired tick. */
    bool updated{false};
    while ((lastProcessedTick < desiredTick)
           && (stateUpdateQueue.size() > 0)) {
        updated = true;
        // Move all NPCs as if their inputs didn't change.
        moveAllNpcs();

        // Check that the processed tick is progressing incrementally.
        NpcStateUpdate& stateUpdate{stateUpdateQueue.front()};
        if (stateUpdate.tickNum != (lastProcessedTick + 1)) {
            LOG_FATAL("Processing NPC movement out of order. "
                      "stateUpdate.tickNum: %u, "
                      "lastProcessedTick: %u",
                      stateUpdate.tickNum, lastProcessedTick);
        }

        // If the update message contained new data, apply it.
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
    // We set our client ahead of the server by an amount equal to our latency,
    // but this means that received messages will appear to be doubly far into
    // the past.
    // To account for this, we double the adjustment before applying.
    // We also negate it since we're reversing the direction.
    tickReplicationOffset += (-2 * adjustment);

    if (tickReplicationOffset >= 0) {
        LOG_FATAL("Adjusted tickReplicationOffset too far into the future. "
                  "offset: %u",
                  tickReplicationOffset);
    }
}

void NpcMovementSystem::receiveEntityUpdates()
{
    /* Process any NPC update messages from the Network. */
    NpcUpdate npcUpdate{};
    while (npcUpdateQueue.pop(npcUpdate)) {
        // Handle the message appropriately.
        switch (npcUpdate.updateType) {
            case NpcUpdateType::ExplicitConfirmation: {
                // If we've been initialized, process the confirmation.
                if (lastReceivedTick != 0) {
                    handleExplicitConfirmation();
                }
                break;
            }
            case NpcUpdateType::ImplicitConfirmation: {
                // If we've been initialized, process the confirmation.
                if (lastReceivedTick != 0) {
                    handleImplicitConfirmation(npcUpdate.tickNum);
                }
                break;
            }
            case NpcUpdateType::Update: {
                handleUpdate(npcUpdate.message);
                break;
            }
        }
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
        // The update message implicitly confirmed all ticks since our last
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
    auto group
        = world.registry.group<Input, Position, PreviousPosition, Movement, BoundingBox, Sprite>(
            entt::exclude<InputHistory>);
    for (entt::entity entity : group) {
        auto [input, position, previousPosition, movement, boundingBox, sprite]
            = group.get<Input, Position, PreviousPosition, Movement, BoundingBox, Sprite>(entity);

        // Save their old position.
        previousPosition = position;

        // Use the current input state to update their velocity for this tick.
        MovementHelpers::updateVelocity(movement, input.inputStates,
                                    SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update their position, using the new velocity.
        MovementHelpers::updatePosition(position, movement,
                                    SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update their bounding box to match the new position.
        boundingBox = Transforms::modelToWorld(sprite.modelBounds, position);

        // TODO: Update their placement in the spatial partition.
    }
}

void NpcMovementSystem::applyUpdateMessage(
    const std::shared_ptr<const EntityUpdate>& entityUpdate)
{
    auto group
        = world.registry.group<Input, Position, PreviousPosition, Movement, BoundingBox, Sprite>(
            entt::exclude<InputHistory>);

    // Use the data in the message to correct any NPCs that changed inputs.
    for (const EntityState& entityState : entityUpdate->entityStates) {
        // Skip the player (not an NPC).
        entt::registry& registry{world.registry};
        entt::entity entity{entityState.entity};
        if (entity == world.playerEntity) {
            continue;
        }

        // Check that the entity exists.
        // TODO: There's possibly an issue here if we receive an EntityUpdate
        //       while the sim happens to be at this system, and the update's
        //       tick is up for processing. We might end up here before
        //       NpcLifetimeSystem was able to construct the entity.
        if (!(registry.valid(entity))) {
            LOG_FATAL("Received update for invalid entity: %u. Message tick: %u", entity,
                entityUpdate->tickNum);
        }

        // Get the entity's components.
        auto [input, position, previousPosition, movement, boundingBox, sprite]
            = group.get<Input, Position, PreviousPosition, Movement, BoundingBox, Sprite>(entity);

        // Apply the received component updates.
        input = entityState.input;
        movement = entityState.movement;
        position = entityState.position;

        // If the previous position hasn't been initialized, set it to the
        // current position so we don't lerp in from the origin.
        if (!(previousPosition.isInitialized)) {
            previousPosition = position;
            previousPosition.isInitialized = true;
        }

        // Move their bounding box to their new position.
        boundingBox = Transforms::modelToWorld(sprite.modelBounds, position);
    }
}

} // namespace Client
} // namespace AM
