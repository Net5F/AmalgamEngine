#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "EntityUpdate.h"
#include "EntityState.h"
#include "Position.h"
#include "Movement.h"
#include "Input.h"
#include "PlayerState.h"
#include "PreviousPosition.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(Simulation& inSim, World& inWorld,
                                           Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
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
    previousPosition.x = currentPosition.x;
    previousPosition.y = currentPosition.y;
    previousPosition.z = currentPosition.z;

    if (!Config::RUN_OFFLINE) {
        // Receive any player entity updates from the server.
        PlayerState& playerState
            = registry.get<PlayerState>(world.playerEntity);
        Uint32 latestReceivedTick
            = processPlayerUpdates(currentPosition, previousPosition,
                                   currentMovement, currentInput, playerState);

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, currentPosition, currentMovement,
                         playerState);

            // Check if there was a mismatch between the positions we had and
            // where the server thought we should be.
            if ((previousPosition.x != currentPosition.x)
                || (previousPosition.y != currentPosition.y)
                || (previousPosition.z != currentPosition.z)) {
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
    Movement& currentMovement, Input& currentInput, PlayerState& playerState)
{
    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick = 0;
    std::shared_ptr<const EntityUpdate> receivedUpdate
        = network.receivePlayerUpdate();
    while (receivedUpdate != nullptr) {
        /* Validate the received tick. */
        Uint32 receivedTick = receivedUpdate->tickNum;

        Uint32 currentTick = sim.getCurrentTick();
        checkReceivedTickValidity(latestReceivedTick, currentTick);

        if (receivedTick > latestReceivedTick) {
            // Track our latest received tick.
            latestReceivedTick = receivedTick;
        }
        else {
            LOG_ERROR("Received ticks out of order. latest: %u, new: %u",
                      latestReceivedTick, receivedTick);
        }

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
        const Movement& receivedMovement = playerUpdate->movement;
        currentMovement.velX = receivedMovement.velX;
        currentMovement.velY = receivedMovement.velY;
        currentMovement.velZ = receivedMovement.velZ;

        const Position& receivedPosition = playerUpdate->position;
        currentPosition.x = receivedPosition.x;
        currentPosition.y = receivedPosition.y;
        currentPosition.z = receivedPosition.z;

        /* Check if the input is mismatched. */
        // Check that the diff is valid.
        Uint32 tickDiff = sim.getCurrentTick() - receivedTick;
        checkTickDiffValidity(tickDiff);

        const Input& receivedInput = playerUpdate->input;
        if (receivedInput.inputStates != playerState.inputHistory[tickDiff]) {
            // Our prediction was wrong, accept the received input and set all
            // inputs in the history after the mismatched input to match it.
            // TODO: This may be incorrect, but it's uncommon and hard to
            // verify. We may want to more carefully overwrite existing inputs.
            currentInput.inputStates = receivedInput.inputStates;
            for (unsigned int i = 0; i <= tickDiff; ++i) {
                playerState.inputHistory[i] = receivedInput.inputStates;
            }

            // Set our old position to the current so we aren't oddly lerping
            // back.
            previousPosition.x = currentPosition.x;
            previousPosition.y = currentPosition.y;
            previousPosition.z = currentPosition.z;
        }

        receivedUpdate = network.receivePlayerUpdate();
    }

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick,
                                        Position& currentPosition,
                                        Movement& currentMovement,
                                        PlayerState& playerState)
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
                                    playerState.inputHistory[tickDiff],
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
    if (tickDiff > (PlayerState::INPUT_HISTORY_LENGTH - 1)) {
        LOG_ERROR("Too few items in the player input history. "
                  "Increase the length or reduce lag. tickDiff: %u, "
                  "historyLength: %u",
                  tickDiff, PlayerState::INPUT_HISTORY_LENGTH);
    }
}

} // namespace Client
} // namespace AM
