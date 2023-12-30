#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Position.h"
#include "Input.h"
#include "InputHistory.h"
#include "PreviousPosition.h"
#include "Rotation.h"
#include "Sprite.h"
#include "Collision.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include "AMAssert.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(Simulation& inSimulation,
                                           World& inWorld, Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, playerMovementUpdateQueue{inNetwork.getEventDispatcher()}
{
}

void PlayerMovementSystem::processMovement()
{
    // Save the old position.
    auto [position, previousPosition]
        = world.registry.get<Position, PreviousPosition>(world.playerEntity);
    previousPosition = position;

    // If we're online, process any updates from the server.
    if (!Config::RUN_OFFLINE) {
        // Apply any player entity updates from the server.
        Uint32 lastUpdateTick{processPlayerUpdates()};

        // If we received updates, replay inputs that came after them.
        if (lastUpdateTick != 0) {
            replayInputs(lastUpdateTick);

            // Check if there was a mismatch between the position we had and
            // where the server thought we should be.
            if (previousPosition != position) {
                printMismatchInfo(lastUpdateTick);
            }
        }
    }

    // Process the player entity's movement for this tick.
    Input& input{world.registry.get<Input>(world.playerEntity)};
    movePlayerEntity(input.inputStates);

    // Signal the updated components to any observers.
    emitUpdateSignals();
}

Uint32 PlayerMovementSystem::processPlayerUpdates()
{
    entt::registry& registry{world.registry};
    auto [input, position, previousPosition, rotation, collision, inputHistory]
        = registry.get<Input, Position, PreviousPosition, Rotation, Collision,
                       InputHistory>(world.playerEntity);

    /* Process any messages for us from the server. */
    PlayerMovementUpdate movementUpdate{};
    Uint32 lastUpdateTick{0};
    while (playerMovementUpdateQueue.pop(movementUpdate)) {
        // Check that the update's tick is in the past.
        Uint32 updateTick{movementUpdate.tickNum};
        Uint32 currentTick{simulation.getCurrentTick()};
        checkReceivedTickValidity(updateTick, currentTick);

        lastUpdateTick = updateTick;

        // Apply the received movement state.
        position = movementUpdate.position;
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);
        collision.worldBounds
            = Transforms::modelToWorldCentered(collision.modelBounds, position);

        // Check that the diff is valid.
        Uint32 tickDiff{simulation.getCurrentTick() - updateTick};
        checkTickDiffValidity(tickDiff);

        // Check if the received input disagrees with what we predicted.
        const Input& receivedInput{movementUpdate.input};
        if (receivedInput.inputStates != inputHistory.inputHistory[tickDiff]) {
            // Our prediction was wrong, accept the received input and set all
            // inputs in the history after the mismatched input to match it.
            // TODO: This may be incorrect, but it's uncommon and hard to
            // verify. We may want to more carefully overwrite existing inputs.
            input.inputStates = receivedInput.inputStates;
            for (unsigned int i = 0; i <= tickDiff; ++i) {
                inputHistory.inputHistory[i] = receivedInput.inputStates;
            }

            // Set our old position to the current so we aren't oddly lerping
            // back.
            previousPosition = position;
        }
    }

    return lastUpdateTick;
}

void PlayerMovementSystem::replayInputs(Uint32 lastUpdateTick)
{
    // Replay all inputs newer than lastUpdateTick, except the current
    // tick's input.
    Uint32 currentTick{simulation.getCurrentTick()};
    auto& inputHistory{world.registry.get<InputHistory>(world.playerEntity)};
    for (Uint32 tickToProcess = (lastUpdateTick + 1);
         tickToProcess < currentTick; ++tickToProcess) {
        // Check that the diff is valid.
        Uint32 tickDiff{currentTick - tickToProcess};
        checkTickDiffValidity(tickDiff);

        // Replay the input state and move the entity.
        movePlayerEntity(inputHistory.inputHistory[tickDiff]);
    }
}

void PlayerMovementSystem::movePlayerEntity(Input::StateArr& inputStates)
{
    auto [position, previousPosition, rotation, collision]
        = world.registry.get<Position, PreviousPosition, Rotation, Collision>(
            world.playerEntity);

    // Calculate their desired next position.
    Position desiredPosition{position};
    desiredPosition = MovementHelpers::calcPosition(
        position, inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

    // Update the direction we're facing, based on our current inputs.
    rotation = MovementHelpers::calcRotation(rotation, inputStates);

    // If we're trying to move, resolve collisions.
    if (desiredPosition != position) {
        // Calculate a new bounding box to match our desired position.
        BoundingBox desiredBounds{Transforms::modelToWorldCentered(
            collision.modelBounds, desiredPosition)};

        // Resolve any collisions with the surrounding bounding boxes.
        BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
            collision.worldBounds, desiredBounds, world.playerEntity,
            world.registry, world.tileMap, world.entityLocator)};

        // Update our collision box and position.
        // Note: Since desiredBounds was properly offset, we can do a simple
        //       diff to get the position.
        position += (resolvedBounds.getMinPosition()
                     - collision.worldBounds.getMinPosition());
        collision.worldBounds = resolvedBounds;
    }

    // If they did actually move, update their position in the locator.
    if (position != previousPosition) {
        world.entityLocator.setEntityLocation(world.playerEntity,
                                              collision.worldBounds);
    }
}

void PlayerMovementSystem::emitUpdateSignals()
{
    // Emit update signals to any observers.
    world.registry.patch<Position>(world.playerEntity, [](auto&) {});
}

void PlayerMovementSystem::printMismatchInfo(Uint32 lastUpdateTick)
{
    auto [position, previousPosition]
        = world.registry.get<Position, PreviousPosition>(world.playerEntity);

    LOG_INFO("Predicted position mismatched after replay: (%.6f, "
             "%.6f) -> (%.6f, %.6f)",
             previousPosition.x, previousPosition.y, position.x, position.y);
    LOG_INFO("lastUpdateTick: %u", lastUpdateTick);
}

void PlayerMovementSystem::checkReceivedTickValidity(
    [[maybe_unused]] Uint32 updateTick, [[maybe_unused]] Uint32 currentTick)
{
    AM_ASSERT((updateTick <= currentTick),
              "Received data for tick %u on tick %u. Server is in the future, "
              "can't replay inputs.",
              updateTick, currentTick);
}

void PlayerMovementSystem::checkTickDiffValidity(Uint32 tickDiff)
{
    // The history includes the current tick, so we only have LENGTH - 1
    // worth of previous data to use (i.e. it's 0-indexed).
    if (tickDiff > (InputHistory::LENGTH - 1)) {
        LOG_FATAL("Too few items in the player input history. "
                  "Increase the length or reduce lag. tickDiff: %u, "
                  "historyLength: %u",
                  tickDiff, InputHistory::LENGTH);
    }
}

} // namespace Client
} // namespace AM
