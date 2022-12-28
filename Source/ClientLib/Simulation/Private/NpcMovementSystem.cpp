#include "NpcMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "MovementUpdate.h"
#include "Name.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Input.h"
#include "Rotation.h"
#include "Collision.h"
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
NpcMovementSystem::NpcMovementSystem(Simulation& inSimulation, World& inWorld,
                                     Network& inNetwork,
                                     SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, npcUpdateQueue{network.getEventDispatcher()}
, spriteData{inSpriteData}
, lastReceivedTick{0}
, lastProcessedTick{0}
{
}

void NpcMovementSystem::updateNpcs()
{
    if (Config::RUN_OFFLINE) {
        // No need to process NPCs if we're running offline.
        return;
    }

    // Receive any updates from the server, update lastReceivedTick.
    receiveMovementUpdates();

    // We want to process updates until we've either processed the desired
    // tick, or have run out of data.
    Uint32 desiredTick{simulation.getReplicationTick()};

    // While we have authoritative data to use, apply updates for all
    // unprocessed ticks including the desired tick.
    bool updated{false};
    while ((lastProcessedTick < desiredTick)
           && (movementUpdateQueue.size() > 0)) {
        updated = true;
        // Move all NPCs as if their inputs didn't change.
        moveAllNpcs();

        // Check that the processed tick is progressing incrementally.
        NpcMovementUpdate& movementUpdate{movementUpdateQueue.front()};
        if (movementUpdate.tickNum != (lastProcessedTick + 1)) {
            LOG_FATAL("Processing NPC movement out of order. "
                      "movementUpdate.tickNum: %u, "
                      "lastProcessedTick: %u",
                      movementUpdate.tickNum, lastProcessedTick);
        }

        // If the update message contained new data, apply it.
        if (movementUpdate.dataChanged) {
            applyUpdateMessage(movementUpdate.movementUpdate);
        }

        lastProcessedTick++;
        movementUpdateQueue.pop();
    }

    // If we're initialized and needed to process a tick but didn't have data,
    // log it.
    if (!updated && (lastReceivedTick != 0)
        && (lastProcessedTick < desiredTick)) {
        LOG_INFO("Tick passed with no npc update. last: %u, desired: %u, "
                 "queueSize: %u",
                 lastProcessedTick, desiredTick, movementUpdateQueue.size());
    }
}

void NpcMovementSystem::receiveMovementUpdates()
{
    // Process any NPC update messages from the Network.
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
    movementUpdateQueue.push({lastReceivedTick, false, nullptr});
}

void NpcMovementSystem::handleImplicitConfirmation(Uint32 confirmedTick)
{
    // If there's a gap > 1 between the latest received tick and the confirmed
    // tick, we know that no changes happened on the in-between ticks and can
    // push confirmations for them.
    Uint32 implicitConfirmations{confirmedTick - lastReceivedTick};
    for (Uint32 i = 1; i <= implicitConfirmations; ++i) {
        movementUpdateQueue.push({(lastReceivedTick + i), false, nullptr});
    }

    lastReceivedTick = confirmedTick;
}

void NpcMovementSystem::handleUpdate(
    const std::shared_ptr<const MovementUpdate>& movementUpdate)
{
    Uint32 newReceivedTick{movementUpdate->tickNum};

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
    movementUpdateQueue.push({newReceivedTick, true, movementUpdate});

    lastReceivedTick = newReceivedTick;
}

void NpcMovementSystem::moveAllNpcs()
{
    auto group = world.registry.group<Input, Position, PreviousPosition,
                                       Velocity, Rotation, Collision>(
        {}, entt::exclude<InputHistory>);
    for (entt::entity entity : group) {
        auto [input, position, previousPosition, velocity, rotation, collision]
            = group.get<Input, Position, PreviousPosition, Velocity, Rotation,
                        Collision>(entity);

        // Save their old position.
        previousPosition = position;

        // Update their velocity for this tick, based on their current inputs.
        velocity = MovementHelpers::updateVelocity(
            velocity, input.inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Calculate their desired position, using the new velocity.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::updatePosition(
            desiredPosition, velocity, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update the direction they're facing, based on their current inputs.
        rotation = MovementHelpers::updateRotation(rotation, input.inputStates);

        // If they're trying to move, resolve collisions.
        if (desiredPosition != position) {
            // Calculate a new bounding box to match their desired position.
            BoundingBox desiredBounds{Transforms::modelToWorldCentered(
                collision.modelBounds, desiredPosition)};

            // Resolve any collisions with the surrounding bounding boxes.
            BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
                collision.worldBounds, desiredBounds, world.tileMap)};

            // Update their collision box and position.
            // Note: Since desiredBounds was properly offset, we can do a
            //       simple diff to get the position.
            position += (resolvedBounds.getMinPosition()
                         - collision.worldBounds.getMinPosition());
            collision.worldBounds = resolvedBounds;
        }
    }
}

void NpcMovementSystem::applyUpdateMessage(
    const std::shared_ptr<const MovementUpdate>& movementUpdate)
{
    auto group = world.registry.group<Input, Position, PreviousPosition,
                                      Velocity, Rotation, Collision>(
        {}, entt::exclude<InputHistory>);

    // Use the data in the message to correct any NPCs that changed inputs.
    for (const MovementState& movementState : movementUpdate->movementStates) {
        // Skip the player (not an NPC).
        entt::registry& registry{world.registry};
        entt::entity entity{movementState.entity};
        if (entity == world.playerEntity) {
            continue;
        }

        // Check that the entity exists.
        // TODO: There's possibly an issue here if we receive a MovementUpdate
        //       while the sim happens to be at this system, and the update's
        //       tick is up for processing. We might end up here before
        //       NpcLifetimeSystem was able to construct the entity.
        if (!(registry.valid(entity))) {
            LOG_FATAL(
                "Received update for invalid entity: %u. Message tick: %u",
                entity, movementUpdate->tickNum);
        }

        // Get the entity's components.
        auto [input, position, previousPosition, velocity, rotation, collision]
            = group.get<Input, Position, PreviousPosition, Velocity, Rotation,
                        Collision>(entity);

        // Apply the received component updates.
        input = movementState.input;
        velocity = movementState.velocity;
        position = movementState.position;
        rotation = movementState.rotation;

        // If the previous position hasn't been initialized, set it to the
        // current position so they don't lerp in from the origin.
        if (!(previousPosition.isInitialized)) {
            previousPosition = position;
            previousPosition.isInitialized = true;
        }

        // Move their collision box to their new position.
        collision.worldBounds
            = Transforms::modelToWorldCentered(collision.modelBounds, position);
    }
}

} // namespace Client
} // namespace AM
