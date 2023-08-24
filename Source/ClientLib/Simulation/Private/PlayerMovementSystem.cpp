#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Position.h"
#include "Velocity.h"
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
#include "Ignore.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(Simulation& inSimulation,
                                           World& inWorld, Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, playerUpdateQueue{inNetwork.getEventDispatcher()}
{
}

void PlayerMovementSystem::processMovement()
{
    // Save the old position.
    Position& position{world.registry.get<Position>(world.playerEntity)};
    PreviousPosition& previousPosition{
        world.registry.get<PreviousPosition>(world.playerEntity)};
    previousPosition = position;

    // If we're online, process any updates from the server.
    Velocity& velocity{world.registry.get<Velocity>(world.playerEntity)};
    Input& input{world.registry.get<Input>(world.playerEntity)};
    Rotation& rotation{world.registry.get<Rotation>(world.playerEntity)};
    Collision& collision{world.registry.get<Collision>(world.playerEntity)};
    if (!Config::RUN_OFFLINE) {
        // Apply any player entity updates from the server.
        InputHistory& inputHistory{
            world.registry.get<InputHistory>(world.playerEntity)};
        Uint32 latestReceivedTick{
            processPlayerUpdates(position, previousPosition, velocity, input,
                                 inputHistory, rotation, collision)};

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, position, velocity, rotation,
                         inputHistory, collision);

            // Check if there was a mismatch between the position we had and
            // where the server thought we should be.
            if (previousPosition != position) {
                LOG_INFO("Predicted position mismatched after replay: (%.6f, "
                         "%.6f) -> (%.6f, %.6f)",
                         previousPosition.x, previousPosition.y, position.x,
                         position.y);
                LOG_INFO("latestReceivedTick: %u", latestReceivedTick);
            }
        }
    }

    // Process the player entity's movement for this tick.
    movePlayerEntity(input.inputStates, velocity, position, rotation,
                     collision);

    // If the player moved, signal it to the UI.
    if (position != previousPosition) {
        world.worldSignals.playerPositionChanged.publish(position);
        // LOG_INFO("Player position: (%.4f, %.4f) tile: (%d, %d)", position.x,
        //          position.y, position.asTilePosition().x,
        //          position.asTilePosition().y);
    }
}

Uint32 PlayerMovementSystem::processPlayerUpdates(
    Position& position, PreviousPosition& previousPosition, Velocity& velocity,
    Input& input, InputHistory& inputHistory, Rotation& rotation,
    Collision& collision)
{
    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick{0};
    std::shared_ptr<const MovementUpdate> receivedUpdate{nullptr};
    while (playerUpdateQueue.pop(receivedUpdate)) {
        /* Validate the received tick. */
        // Check that the received tick is in the past.
        Uint32 receivedTick{receivedUpdate->tickNum};
        Uint32 currentTick{simulation.getCurrentTick()};
        checkReceivedTickValidity(receivedTick, currentTick);

        // Check that the received tick is ahead of our latest.
        AM_ASSERT((receivedTick > latestReceivedTick),
                  "Received ticks out of order. latest: %u, new: %u",
                  latestReceivedTick, receivedTick);

        // Track our latest received tick.
        latestReceivedTick = receivedTick;

        /* Find the player data. */
        const std::vector<MovementState>& entities{
            receivedUpdate->movementStates};
        const MovementState* playerUpdate{nullptr};
        for (auto entityIt = entities.begin(); entityIt != entities.end();
             ++entityIt) {
            if (entityIt->entity == world.playerEntity) {
                playerUpdate = &(*entityIt);
                break;
            }
        }

        AM_ASSERT((playerUpdate != nullptr),
                  "Failed to find player entity in a message that should have "
                  "contained one.");

        /* Apply the received movement state. */
        velocity = playerUpdate->velocity;
        position = playerUpdate->position;
        rotation = playerUpdate->rotation;
        collision.worldBounds
            = Transforms::modelToWorldCentered(collision.modelBounds, position);

        /* Check if the input is mismatched. */
        // Check that the diff is valid.
        Uint32 tickDiff{simulation.getCurrentTick() - receivedTick};
        checkTickDiffValidity(tickDiff);

        // Check if the received input disagrees with what we predicted.
        const Input& receivedInput{playerUpdate->input};
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

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick,
                                        Position& position, Velocity& velocity,
                                        Rotation& rotation,
                                        InputHistory& inputHistory,
                                        Collision& collision)
{
    Uint32 currentTick{simulation.getCurrentTick()};
    checkReceivedTickValidity(latestReceivedTick, currentTick);

    // Replay all inputs newer than latestReceivedTick, except the current
    // tick's input.
    for (Uint32 tickToProcess = (latestReceivedTick + 1);
         tickToProcess < currentTick; ++tickToProcess) {
        // Check that the diff is valid.
        Uint32 tickDiff{currentTick - tickToProcess};
        checkTickDiffValidity(tickDiff);

        // Replay the input state and move the entity.
        movePlayerEntity(inputHistory.inputHistory[tickDiff], velocity,
                         position, rotation, collision);
    }
}

void PlayerMovementSystem::checkReceivedTickValidity(Uint32 receivedTick,
                                                     Uint32 currentTick)
{
    AM_ASSERT((receivedTick <= currentTick),
              "Received data for tick %u on tick %u. Server is in the future, "
              "can't replay inputs.",
              receivedTick, currentTick);
    ignore(receivedTick);
    ignore(currentTick);
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

void PlayerMovementSystem::movePlayerEntity(Input::StateArr& inputStates,
                                            Velocity& velocity,
                                            Position& position,
                                            Rotation& rotation,
                                            Collision& collision)
{
    // Update our velocity for this tick, based on our current inputs.
    velocity = MovementHelpers::updateVelocity(
        velocity, inputStates, SharedConfig::SIM_TICK_TIMESTEP_S);

    // Calculate our desired position, using the new velocity.
    Position desiredPosition{position};
    desiredPosition = MovementHelpers::updatePosition(
        position, velocity, SharedConfig::SIM_TICK_TIMESTEP_S);

    // Update the direction we're facing, based on our current inputs.
    rotation = MovementHelpers::updateRotation(rotation, inputStates);

    // If we're trying to move, resolve collisions.
    if (desiredPosition != position) {
        // Calculate a new bounding box to match our desired position.
        BoundingBox desiredBounds{Transforms::modelToWorldCentered(
            collision.modelBounds, desiredPosition)};

        // Resolve any collisions with the surrounding bounding boxes.
        BoundingBox resolvedBounds{MovementHelpers::resolveCollisions(
            collision.worldBounds, desiredBounds, world.tileMap)};

        // Update our collision box and position.
        // Note: Since desiredBounds was properly offset, we can do a simple
        //       diff to get the position.
        position += (resolvedBounds.getMinPosition()
                     - collision.worldBounds.getMinPosition());
        collision.worldBounds = resolvedBounds;
    }
}

} // namespace Client
} // namespace AM
