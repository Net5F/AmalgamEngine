#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "EntityUpdate.h"
#include "ClientNetworkDefs.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(Game& inGame, World& inWorld,
                                           Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
{
}

void PlayerMovementSystem::processMovements()
{
    EntityID playerID = world.playerData.ID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    // Save the old position.
    PositionComponent& oldPosition = world.oldPositions[playerID];
    oldPosition.x = currentPosition.x;
    oldPosition.y = currentPosition.y;

    if (!RUN_OFFLINE) {
        // Receive any player entity updates from the server.
        Uint32 latestReceivedTick = processPlayerUpdates();

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick);

            // Check if there was a mismatch between the positions we had and
            // where the server thought we should be.
            if (oldPosition.x != currentPosition.x
                || oldPosition.y != currentPosition.y) {
                LOG_INFO("Predicted position mismatched after replay: (%.6f, "
                         "%.6f) -> (%.6f, %.6f)",
                         oldPosition.x, oldPosition.y, currentPosition.x,
                         currentPosition.y);
                LOG_INFO("latestReceivedTick: %u", latestReceivedTick);
            }
        }
    }

    // Use the current input state to update movement for this tick.
    MovementHelpers::moveEntity(currentPosition, currentMovement,
                                world.inputs[playerID].inputStates,
                                GAME_TICK_TIMESTEP_S);
}

Uint32 PlayerMovementSystem::processPlayerUpdates()
{
    EntityID playerID = world.playerData.ID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick = 0;
    std::shared_ptr<const EntityUpdate> receivedUpdate
        = network.receivePlayerUpdate();
    while (receivedUpdate != nullptr) {
        /* Validate the received tick. */
        Uint32 receivedTick = receivedUpdate->tickNum;

        Uint32 currentTick = game.getCurrentTick();
        checkReceivedTickValidity(latestReceivedTick, currentTick);

        if (receivedTick > latestReceivedTick) {
            // Track our latest received tick.
            latestReceivedTick = receivedTick;
        }
        else {
            LOG_ERROR("Received ticks out of order. latest: %u, new: %u",
                      latestReceivedTick, receivedTick);
        }

        // Pull out the vector of entities.
        const std::vector<Entity>& entities = receivedUpdate->entities;

        // Find the player data.
        const Entity* playerUpdate = nullptr;
        for (auto entityIt = entities.begin(); entityIt != entities.end();
             ++entityIt) {
            if (entityIt->id == playerID) {
                playerUpdate = &(*entityIt);
                break;
            }
        }

        if (playerUpdate == nullptr) {
            LOG_ERROR("Failed to find player entity in a message that should "
                      "have contained one.");
        }

        /* Update the movements. */
        const MovementComponent& receivedMovement
            = playerUpdate->movementComponent;
        currentMovement.velX = receivedMovement.velX;
        currentMovement.velY = receivedMovement.velY;

        /* Move to the received position. */
        const PositionComponent& receivedPosition
            = playerUpdate->positionComponent;
        currentPosition.x = receivedPosition.x;
        currentPosition.y = receivedPosition.y;

        /** Check if the input is mismatched. */
        // Check that the diff is valid.
        Uint32 tickDiff = game.getCurrentTick() - receivedTick;
        checkTickDiffValidity(tickDiff);

        const InputComponent& receivedInput = playerUpdate->inputComponent;
        if (receivedInput.inputStates
            != world.playerData.inputHistory[tickDiff]) {
            // Our prediction was wrong, accept the received input and set all
            // inputs in the history after the mismatched input to match it.
            world.inputs[playerID].inputStates = receivedInput.inputStates;
            for (unsigned int i = 0; i <= tickDiff; ++i) {
                world.playerData.inputHistory[i] = receivedInput.inputStates;
            }

            // Set our old position to the current so we aren't oddly lerping
            // back.
            PositionComponent& oldPosition = world.oldPositions[playerID];
            oldPosition.x = currentPosition.x;
            oldPosition.y = currentPosition.y;
        }

        receivedUpdate = network.receivePlayerUpdate();
    }

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick)
{
    EntityID playerID = world.playerData.ID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    Uint32 currentTick = game.getCurrentTick();
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
                                    world.playerData.inputHistory[tickDiff],
                                    GAME_TICK_TIMESTEP_S);
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
    if (tickDiff > (PlayerData::INPUT_HISTORY_LENGTH - 1)) {
        LOG_ERROR("Too few items in the player input history. "
                  "Increase the length or reduce lag. tickDiff: %u, "
                  "historyLength: %u",
                  tickDiff, PlayerData::INPUT_HISTORY_LENGTH);
    }
}

} // namespace Client
} // namespace AM
