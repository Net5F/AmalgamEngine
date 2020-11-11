#include "NpcMovementSystem.h"
#include "MovementHelpers.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "ClientNetworkDefs.h"
#include "EntityUpdate.h"
#include "Log.h"
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
}

void NpcMovementSystem::updateNpcs()
{
    if (RUN_OFFLINE) {
        // No need to process NPCs if we're running offline.
        return;
    }

    // Receive any updates from the server, update pastTickOffset.
    receiveEntityUpdates();

    // We want to process updates until we've either hit the desired, or run out
    // of data.
    Uint32 desiredTick = game.getCurrentTick() - tickReplicationOffset;

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

    if ((lastReceivedTick != 0) && !updated) {
        LOG_INFO(
            "Tick passed with no update. last: %u, desired: %u, queueSize: %u",
            lastProcessedTick, desiredTick, stateUpdateQueue.size());
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
    for (std::size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        /* Move all NPCs that have an input, position, and movement component.
         */
        if (entityID != world.playerID
            && (world.componentFlags[entityID] & ComponentFlag::Input)
            && (world.componentFlags[entityID] & ComponentFlag::Position)
            && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Save the old position.
            PositionComponent& currentPosition = world.positions[entityID];
            PositionComponent& oldPosition = world.oldPositions[entityID];
            oldPosition.x = currentPosition.x;
            oldPosition.y = currentPosition.y;

            // Process their movement.
            MovementHelpers::moveEntity(
                currentPosition, world.movements[entityID],
                world.inputs[entityID].inputStates, GAME_TICK_TIMESTEP_S);
        }
    }
}

void NpcMovementSystem::applyUpdateMessage(
    const std::shared_ptr<const EntityUpdate>& entityUpdate)
{
    const std::vector<Entity>& entities = entityUpdate->entities;
    /* Use the data in the message to correct any NPCs that did change inputs.
     */
    for (auto entityIt = entities.begin(); entityIt != entities.end();
         ++entityIt) {
        // Skip the player (not an NPC).
        EntityID entityID = entityIt->id;
        if (entityID == world.playerID) {
            continue;
        }

        // If the entity doesn't exist, add it to our list.
        if (!(world.entityExists(entityID))) {
            LOG_INFO("New entity added. ID: %u", entityID);
            // TODO: Add names for real.
            world.addEntity(std::to_string(entityID), entityID);

            // TODO: Get this info from the server.
            // Get the same texture as the player.
            world.sprites[entityID].texturePtr
                = world.sprites[world.playerID].texturePtr;
            world.sprites[entityID].posInTexture
                = world.sprites[world.playerID].posInTexture;
            world.sprites[entityID].width = 64;
            world.sprites[entityID].height = 64;

            world.attachComponent(entityID, ComponentFlag::Input);
            world.attachComponent(entityID, ComponentFlag::Movement);
            world.attachComponent(entityID, ComponentFlag::Position);
            world.attachComponent(entityID, ComponentFlag::Sprite);

            // Init their old position so they don't lerp in from elsewhere.
            world.oldPositions[entityID] = entityIt->positionComponent;
        }

        // Update the inputs.
        world.inputs[entityID] = entityIt->inputComponent;

        // Update the movements.
        world.movements[entityID] = entityIt->movementComponent;

        // Update the currentPosition.
        // TEMP
        //        const PositionComponent& currentPosition =
        //        world.positions[entityID]; const PositionComponent&
        //        newPosition = entityIt->positionComponent; LOG_INFO("Update:
        //        %d: (%f, %f) -> (%f, %f)", entityID,
        //                 currentPosition.x, currentPosition.y, newPosition.x,
        //                 newPosition.y);
        // TEMP
        world.positions[entityID] = entityIt->positionComponent;
    }
}

} // namespace Client
} // namespace AM
