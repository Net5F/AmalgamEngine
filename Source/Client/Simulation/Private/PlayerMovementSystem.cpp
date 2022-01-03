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
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(
    Simulation& inSim, World& inWorld,
    EventDispatcher& inNetworkEventDispatcher)
: sim{inSim}
, world{inWorld}
, playerUpdateQueue{inNetworkEventDispatcher}
{
}

void PlayerMovementSystem::processMovement()
{
    // Save the old position.
    Position& position{world.registry.get<Position>(world.playerEntity)};
    PreviousPosition& previousPosition{world.registry.get<PreviousPosition>(world.playerEntity)};
    previousPosition = position;

    // If we're online, process any updates from the server.
    Velocity& velocity{world.registry.get<Velocity>(world.playerEntity)};
    Input& input{world.registry.get<Input>(world.playerEntity)};
    if (!Config::RUN_OFFLINE) {
        // Apply any player entity updates from the server.
        InputHistory& inputHistory{world.registry.get<InputHistory>(world.playerEntity)};
        Uint32 latestReceivedTick{processPlayerUpdates(position, previousPosition, velocity, input, inputHistory)};

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, position, velocity,
                         inputHistory);

            // Check if there was a mismatch between the position we had and
            // where the server thought we should be.
            if (previousPosition != position) {
                LOG_INFO("Predicted position mismatched after replay: (%.6f, "
                         "%.6f) -> (%.6f, %.6f)",
                         previousPosition.x, previousPosition.y,
                         position.x, position.y);
                LOG_INFO("latestReceivedTick: %u", latestReceivedTick);
            }
        }
    }

    // Use the current input state to update our velocity for this tick.
    MovementHelpers::updateVelocity(velocity, input.inputStates,
                                SharedConfig::SIM_TICK_TIMESTEP_S);

    // Update our position, using the new velocity.
    MovementHelpers::updatePosition(position, velocity,
                                SharedConfig::SIM_TICK_TIMESTEP_S);

    // Update our bounding box to match the new position.
    Sprite& sprite{world.registry.get<Sprite>(world.playerEntity)};
    BoundingBox& boundingBox{world.registry.get<BoundingBox>(world.playerEntity)};
    boundingBox = Transforms::modelToWorld(sprite.modelBounds, position);
}

Uint32 PlayerMovementSystem::processPlayerUpdates(
    Position& position, PreviousPosition& previousPosition,
    Velocity& velocity, Input& input, InputHistory& inputHistory)
{
    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick{0};
    std::shared_ptr<const MovementUpdate> receivedUpdate{nullptr};
    while (playerUpdateQueue.pop(receivedUpdate)) {
        /* Validate the received tick. */
        // Check that the received tick is in the past.
        Uint32 receivedTick{receivedUpdate->tickNum};
        Uint32 currentTick{sim.getCurrentTick()};
        checkReceivedTickValidity(receivedTick, currentTick);

        // Check that the received tick is ahead of our latest.
        if (receivedTick <= latestReceivedTick) {
            LOG_FATAL("Received ticks out of order. latest: %u, new: %u",
                      latestReceivedTick, receivedTick);
        }

        // Track our latest received tick.
        latestReceivedTick = receivedTick;

        /* Find the player data. */
        const std::vector<MovementState>& entities{receivedUpdate->movementStates};
        const MovementState* playerUpdate{nullptr};
        for (auto entityIt = entities.begin(); entityIt != entities.end();
             ++entityIt) {
            if (entityIt->entity == world.playerEntity) {
                playerUpdate = &(*entityIt);
                break;
            }
        }

        if (playerUpdate == nullptr) {
            LOG_FATAL("Failed to find player entity in a message that should "
                      "have contained one.");
        }

        /* Apply the received velocity and position. */
        velocity = playerUpdate->velocity;
        position = playerUpdate->position;

        /* Check if the input is mismatched. */
        // Check that the diff is valid.
        Uint32 tickDiff{sim.getCurrentTick() - receivedTick};
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
                                        Position& position,
                                        Velocity& velocity,
                                        InputHistory& inputHistory)
{
    Uint32 currentTick{sim.getCurrentTick()};
    checkReceivedTickValidity(latestReceivedTick, currentTick);

    /* Replay all inputs newer than latestReceivedTick, except the current
       tick's input. */
    for (Uint32 tickToProcess = (latestReceivedTick + 1);
         tickToProcess < currentTick; ++tickToProcess) {
        // Check that the diff is valid.
        Uint32 tickDiff{currentTick - tickToProcess};
        checkTickDiffValidity(tickDiff);

        // Use the appropriate input state to update our velocity.
        MovementHelpers::updateVelocity(velocity,
                                    inputHistory.inputHistory[tickDiff],
                                    SharedConfig::SIM_TICK_TIMESTEP_S);

        // Update our position, using the new velocity.
        MovementHelpers::updatePosition(position, velocity,
                                    SharedConfig::SIM_TICK_TIMESTEP_S);
    }
}

void PlayerMovementSystem::checkReceivedTickValidity(Uint32 receivedTick,
                                                     Uint32 currentTick)
{
    if (receivedTick > currentTick) {
        LOG_FATAL("Received data for tick %u on tick %u. Server is in the "
                  "future, can't replay inputs.",
                  receivedTick, currentTick);
    }
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
