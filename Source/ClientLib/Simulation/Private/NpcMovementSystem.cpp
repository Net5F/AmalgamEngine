#include "NpcMovementSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "MovementHelpers.h"
#include "NpcMovementUpdate.h"
#include "Name.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Input.h"
#include "Rotation.h"
#include "Collision.h"
#include "InputHistory.h"
#include "Transforms.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include <memory>

namespace AM
{
namespace Client
{
NpcMovementSystem::NpcMovementSystem(Simulation& inSimulation, World& inWorld,
                                     Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, npcMovementUpdateQueue{network.getEventDispatcher()}
, lastProcessedTick{0}
{
}

// TODO: Where do we want to patch/replace to trigger signals?
void NpcMovementSystem::updateNpcs()
{
    if (Config::RUN_OFFLINE) {
        // No need to process NPCs if we're running offline.
        return;
    }

    // While we haven't reached the desired tick and have more data to process.
    Uint32 desiredTick{simulation.getReplicationTick()};
    Uint32 lastReceivedTick{network.getLastReceivedTick()};
    bool updated{false};
    while ((lastProcessedTick < desiredTick)
           && (lastProcessedTick < lastReceivedTick)) {
        updated = true;

        // Move all NPCs as if their inputs didn't change.
        moveAllNpcs();

        // If we've received updates for this tick, process them.
        const NpcMovementUpdate* npcMovementUpdate{
            npcMovementUpdateQueue.peek()};
        while ((npcMovementUpdate != nullptr)
               && (npcMovementUpdate->tickNum == (lastProcessedTick + 1))) {
            applyUpdateMessage(*npcMovementUpdate);

            npcMovementUpdateQueue.pop();
            npcMovementUpdate = npcMovementUpdateQueue.peek();
        }

        // TEMP
        if ((npcMovementUpdate != nullptr)
            && (npcMovementUpdate->tickNum < (lastProcessedTick + 1))) {
            LOG_INFO("Old tick: %u", npcMovementUpdate->tickNum);
        }
        // TEMP

        lastProcessedTick++;
    }

    if (!updated) {
        LOG_INFO(
            "Tick passed with no npc update. lastProcessed: %u, desired: %u, "
            "lastReceived: %u, queueSize: %u",
            lastProcessedTick, desiredTick, lastReceivedTick,
            npcMovementUpdateQueue.size());
    }
}

void NpcMovementSystem::moveAllNpcs()
{
    auto movementGroup = world.registry.group<Input, Position, PreviousPosition,
                                              Velocity, Rotation, Collision>(
        entt::get<Sprite>, entt::exclude<InputHistory>);
    for (auto [entity, input, position, previousPosition, velocity, rotation,
               collision, sprite] : movementGroup.each()) {
        // Save their old position.
        previousPosition = position;

        // Update their velocity for this tick, based on their current inputs.
        velocity = MovementHelpers::updateVelocity(
            velocity, input.inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Calculate their desired position, using the new velocity.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::updatePosition(
            position, velocity, SharedConfig::SIM_TICK_TIMESTEP_S);

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
    const NpcMovementUpdate& npcMovementUpdate)
{
    entt::registry& registry{world.registry};
    auto movementGroup = registry.group<Input, Position, PreviousPosition,
                                        Velocity, Rotation, Collision>(
        entt::get<Sprite>, entt::exclude<InputHistory>);

    // Check that the entity exists.
    // TODO: There's possibly an issue here if we receive a MovementUpdate
    //       while the sim happens to be at this system, and the update's
    //       tick is up for processing. We might end up here before
    //       EntityLifetimeSystem was able to construct the entity.
    entt::entity entity{npcMovementUpdate.entity};
    if (!(world.entityIDIsInUse(entity))) {
        LOG_FATAL(
            "Received update for invalid entity: %u. Message tick: %u",
            entity, npcMovementUpdate.tickNum);
    }

    // Get the entity's components.
    auto [input, position, previousPosition, velocity, rotation, collision]
        = movementGroup.get<Input, Position, PreviousPosition, Velocity,
                            Rotation, Collision>(entity);

    // Apply the received component updates.
    input = npcMovementUpdate.input;
    position = npcMovementUpdate.position;
    velocity = npcMovementUpdate.velocity;
    rotation = npcMovementUpdate.rotation;

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

} // namespace Client
} // namespace AM
