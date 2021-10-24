#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Position.h"
#include "Movement.h"
#include "Input.h"
#include "InputHistory.h"
#include "PreviousPosition.h"
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
: sim(inSim)
, world(inWorld)
, playerUpdateQueue(inNetworkEventDispatcher)
{
}

void PlayerMovementSystem::processMovements()
{
    entt::registry& registry = world.registry;
    Position& currentPosition = registry.get<Position>(world.playerEntity);
    Movement& currentMovement = registry.get<Movement>(world.playerEntity);
    Input& currentInput = registry.get<Input>(world.playerEntity);

    // Save the old position.
    PreviousPosition& previousPosition
        = registry.get<PreviousPosition>(world.playerEntity);
    previousPosition = currentPosition;

    if (!Config::RUN_OFFLINE) {
        // Apply any player entity updates from the server.
        InputHistory& inputHistory
            = registry.get<InputHistory>(world.playerEntity);
        Uint32 latestReceivedTick
            = processPlayerUpdates(currentPosition, previousPosition,
                                   currentMovement, currentInput, inputHistory);

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, currentPosition, currentMovement,
                         inputHistory);

            // Check if there was a mismatch between the position we had and
            // where the server thought we should be.
            if (previousPosition != currentPosition) {
                LOG_INFO("Predicted position mismatched after replay: (%.6f, "
                         "%.6f) -> (%.6f, %.6f)",
                         previousPosition.x, previousPosition.y,
                         currentPosition.x, currentPosition.y);
                LOG_INFO("latestReceivedTick: %u", latestReceivedTick);
            }
        }
    }

    // Use the current input state to update movement for this tick.
    MovementHelpers::moveEntity(currentPosition, currentMovement,
                                currentInput.inputStates,
                                SharedConfig::SIM_TICK_TIMESTEP_S);
}

Uint32 PlayerMovementSystem::processPlayerUpdates(
    Position& currentPosition, PreviousPosition& previousPosition,
    Movement& currentMovement, Input& currentInput, InputHistory& inputHistory)
{
    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick = 0;
    std::shared_ptr<const EntityUpdate> receivedUpdate{nullptr};
    while (playerUpdateQueue.pop(receivedUpdate)) {
        /* Validate the received tick. */
        // Check that the received tick is in the past.
        Uint32 receivedTick = receivedUpdate->tickNum;
        Uint32 currentTick = sim.getCurrentTick();
        checkReceivedTickValidity(receivedTick, currentTick);

        // Check that the received tick is ahead of our latest.
        if (receivedTick <= latestReceivedTick) {
            LOG_ERROR("Received ticks out of order. latest: %u, new: %u",
                      latestReceivedTick, receivedTick);
        }

        // Track our latest received tick.
        latestReceivedTick = receivedTick;

        /* Find the player data. */
        const std::vector<EntityState>& entities = receivedUpdate->entityStates;
        const EntityState* playerUpdate = nullptr;
        for (auto entityIt = entities.begin(); entityIt != entities.end();
             ++entityIt) {
            if (entityIt->entity == world.playerEntity) {
                playerUpdate = &(*entityIt);
                break;
            }
        }

        if (playerUpdate == nullptr) {
            LOG_ERROR("Failed to find player entity in a message that should "
                      "have contained one.");
        }

        /* Apply the received movement and position. */
        currentMovement = playerUpdate->movement;
        currentPosition = playerUpdate->position;

        /* Check if the input is mismatched. */
        // Check that the diff is valid.
        Uint32 tickDiff = sim.getCurrentTick() - receivedTick;
        checkTickDiffValidity(tickDiff);

        // Check if the received input disagrees with what we predicted.
        const Input& receivedInput = playerUpdate->input;
        if (receivedInput.inputStates != inputHistory.inputHistory[tickDiff]) {
            // Our prediction was wrong, accept the received input and set all
            // inputs in the history after the mismatched input to match it.
            // TODO: This may be incorrect, but it's uncommon and hard to
            // verify. We may want to more carefully overwrite existing inputs.
            currentInput.inputStates = receivedInput.inputStates;
            for (unsigned int i = 0; i <= tickDiff; ++i) {
                inputHistory.inputHistory[i] = receivedInput.inputStates;
            }

            // Set our old position to the current so we aren't oddly lerping
            // back.
            previousPosition = currentPosition;
        }
    }

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick,
                                        Position& currentPosition,
                                        Movement& currentMovement,
                                        InputHistory& inputHistory)
{
    Uint32 currentTick = sim.getCurrentTick();
    checkReceivedTickValidity(latestReceivedTick, currentTick);

    /* Replay all inputs newer than latestReceivedTick, except the current
       tick's input. */
    for (Uint32 tickToProcess = (latestReceivedTick + 1);
         tickToProcess < currentTick; ++tickToProcess) {
        // Check that the diff is valid.
        Uint32 tickDiff = currentTick - tickToProcess;
        checkTickDiffValidity(tickDiff);

        // Use the appropriate input state to update movement.
        MovementHelpers::moveEntity(currentPosition, currentMovement,
                                    inputHistory.inputHistory[tickDiff],
                                    SharedConfig::SIM_TICK_TIMESTEP_S);
    }
}

void PlayerMovementSystem::checkReceivedTickValidity(Uint32 receivedTick,
                                                     Uint32 currentTick)
{
    if (receivedTick > currentTick) {
        LOG_ERROR("Received data for tick %u on tick %u. Server is in the "
                  "future, can't replay inputs.",
                  receivedTick, currentTick);
    }
}

void PlayerMovementSystem::checkTickDiffValidity(Uint32 tickDiff)
{
    // The history includes the current tick, so we only have LENGTH - 1
    // worth of previous data to use (i.e. it's 0-indexed).
    if (tickDiff > (InputHistory::LENGTH - 1)) {
        LOG_ERROR("Too few items in the player input history. "
                  "Increase the length or reduce lag. tickDiff: %u, "
                  "historyLength: %u",
                  tickDiff, InputHistory::LENGTH);
    }
}

} // namespace Client
} // namespace AM
