#include "NpcMovementSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "MovementHelpers.h"
#include "MovementUpdate.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
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

void NpcMovementSystem::updateNpcs()
{
    if (Config::RUN_OFFLINE) {
        // No need to process NPCs if we're running offline.
        return;
    }

    // If this is our first run, initialize our lastProcessedTick.
    if (lastProcessedTick == 0) {
        initLastProcessedTick();
    }

    // While we haven't reached the desired tick and have more data to process.
    Uint32 desiredTick{simulation.getReplicationTick()};
    Uint32 lastReceivedTick{network.getLastReceivedTick()};
    while ((lastProcessedTick < desiredTick)
           && (lastProcessedTick < lastReceivedTick)) {
        // Move all NPCs as if their inputs didn't change.
        moveAllNpcs();

        // If there's an update message waiting.
        std::shared_ptr<const MovementUpdate>* npcMovementUpdate{
            npcMovementUpdateQueue.peek()};
        if ((npcMovementUpdate != nullptr)) {
            Uint32 messageTick{(*npcMovementUpdate)->tickNum};
            AM_ASSERT((messageTick >= (lastProcessedTick + 1)),
                      "Processed NPC movement out of order.");

            // If the update is for this tick, apply it.
            if (messageTick == (lastProcessedTick + 1)) {
                applyUpdateMessage(*(npcMovementUpdate->get()));
                npcMovementUpdateQueue.pop();
            }
        }

        lastProcessedTick++;
    }

    // Signal the updated components to any observers.
    emitUpdateSignals();

    // If we wanted to process more ticks but ran out of data, log it.
    if (lastProcessedTick < desiredTick) {
        LOG_INFO("Ran out of NPC data. lastProcessed: %u, desired: %u, "
                 "lastReceived: %u, queueSize: %u",
                 lastProcessedTick, desiredTick, lastReceivedTick,
                 npcMovementUpdateQueue.size());
    }
}

void NpcMovementSystem::initLastProcessedTick()
{
    // Since we aren't ran until a server connection is established, the sim's
    // currentTick is set to what the server gave us in ConnectionResponse.
    // Our first received updates will be for that tick, so we init to
    // currentTick - 1 to prepare for processing them.
    lastProcessedTick = simulation.getReplicationTick() - 1;
}

void NpcMovementSystem::moveAllNpcs()
{
    auto movementGroup = world.registry.group<Input, Position, PreviousPosition,
                                              Movement, Rotation, Collision>(
        entt::get<GraphicState>, entt::exclude<InputHistory>);
    for (auto [entity, input, position, previousPosition, movement, rotation,
               collision, graphicState] : movementGroup.each()) {
        // Save their old position.
        previousPosition = position;

        // Calculate their desired next position.
        Position desiredPosition{position};
        desiredPosition = MovementHelpers::calcPosition(
            position, input.inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update the direction they're facing, based on their current inputs.
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);

        // If they're trying to move, resolve collisions.
        if (desiredPosition != position) {
            // Calculate a new bounding box to match their desired position.
            BoundingBox desiredBounds{Transforms::modelToWorldCentered(
                collision.modelBounds, desiredPosition)};

            // Resolve any collisions with the surrounding bounding boxes.
            BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
                collision.worldBounds, desiredBounds, entity, world.registry,
                world.tileMap, world.entityLocator)};

            // Update their collision box and position.
            // Note: Since desiredBounds was properly offset, we can do a
            //       simple diff to get the position.
            position += (resolvedBounds.getMinPosition()
                         - collision.worldBounds.getMinPosition());
            collision.worldBounds = resolvedBounds;
        }

        // If they did actually move, update their position in the locator.
        if (position != previousPosition) {
            world.entityLocator.setEntityLocation(entity,
                                                  collision.worldBounds);
        }
    }
}

void NpcMovementSystem::applyUpdateMessage(
    const MovementUpdate& npcMovementUpdate)
{
    entt::registry& registry{world.registry};
    auto movementGroup = registry.group<Input, Position, PreviousPosition,
                                        Movement, Rotation, Collision>(
        entt::get<GraphicState>, entt::exclude<InputHistory>);

    // Apply each updated entity's new state.
    for (const MovementState& movementState :
         npcMovementUpdate.movementStates) {
        // Check that the entity exists.
        // TODO: There's possibly an issue here if we receive a MovementUpdate
        //       while the sim happens to be at this system, and the update's
        //       tick is up for processing. We might end up here before
        //       EntityLifetimeSystem was able to construct the entity.
        entt::entity entity{movementState.entity};
        if (!(registry.valid(entity))) {
            LOG_FATAL(
                "Received update for invalid entity: %u. Message tick: %u",
                entity, npcMovementUpdate.tickNum);
        }

        // Get the entity's components.
        auto [input, position, previousPosition, movement, rotation, collision]
            = movementGroup.get<Input, Position, PreviousPosition, Movement,
                                Rotation, Collision>(entity);

        // Apply the received component updates.
        input = movementState.input;
        position = movementState.position;
        movement.velocityZ = movementState.movementVelocityZ;
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);

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

void NpcMovementSystem::emitUpdateSignals()
{
    // Emit update signals to any observers.
    auto view{world.registry.view<Position, PreviousPosition>()};
    for (entt::entity entity : view) {
        world.registry.patch<Position>(entity, [](auto&) {});
    }
}

} // namespace Client
} // namespace AM
