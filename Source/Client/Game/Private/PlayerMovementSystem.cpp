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
    EntityID playerID = world.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    // Save the old position.
    PositionComponent& oldPosition = world.oldPositions[playerID];
    oldPosition.x = currentPosition.x;
    oldPosition.y = currentPosition.y;

    if (!RUN_OFFLINE) {
        // Receive any player entity updates from the server.
        Uint32 latestReceivedTick = processReceivedUpdates(
            playerID, currentPosition, currentMovement);

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, currentPosition, currentMovement);

            // Check if there was a mismatch between the positions we had and
            // where the server thought we should be.
            if (oldPosition.x != currentPosition.x
                || oldPosition.y != currentPosition.y) {
                LOG_INFO("Predicted position mismatched after replay: ({:.6f}, "
                         "{:.6f}) -> ({:.6f}, {:.6f})",
                         oldPosition.x, oldPosition.y, currentPosition.x,
                         currentPosition.y);
                LOG_INFO("latestReceivedTick: {}", latestReceivedTick);
            }
        }
    }

    // Use the current input state to update movement for this tick.
    MovementHelpers::moveEntity(currentPosition, currentMovement,
                                world.inputs[playerID].inputStates,
                                GAME_TICK_TIMESTEP_S);
}

Uint32 PlayerMovementSystem::processReceivedUpdates(
    EntityID playerID, PositionComponent& currentPosition,
    MovementComponent& currentMovement)
{
    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick = 0;
    std::shared_ptr<const EntityUpdate> receivedUpdate
        = network.receivePlayerUpdate();
    while (receivedUpdate != nullptr) {
        // Track our latest received tick.
        Uint32 newTick = receivedUpdate->tickNum;
        if (newTick > latestReceivedTick) {
            latestReceivedTick = newTick;
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
        const MovementComponent& newMovement = playerUpdate->movementComponent;
        currentMovement.velX = newMovement.velX;
        currentMovement.velY = newMovement.velY;

        /* Move to the received position. */
        const PositionComponent& receivedPosition
            = playerUpdate->positionComponent;
        currentPosition.x = receivedPosition.x;
        currentPosition.y = receivedPosition.y;

        receivedUpdate = network.receivePlayerUpdate();
    }

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick,
                                        PositionComponent& currentPosition,
                                        MovementComponent& currentMovement)
{
    Uint32 currentTick = game.getCurrentTick();
    if (latestReceivedTick > currentTick) {
        LOG_ERROR("Received data for tick {} on tick {}. Server is in the "
                  "future, can't replay "
                  "inputs.",
                  latestReceivedTick, currentTick);
    }

    /* Replay all inputs since the received message, except the current. */
    for (Uint32 i = (latestReceivedTick + 1); i < currentTick; ++i) {
        Uint32 tickDiff = currentTick - i;

        if (tickDiff > World::INPUT_HISTORY_LENGTH) {
            LOG_ERROR("Too few items in the player input history. "
                      "Increase the length or reduce lag. tickDiff: {}, "
                      "historyLength: {}",
                      tickDiff, World::INPUT_HISTORY_LENGTH);
        }

        // Use the appropriate input state to update movement.
        MovementHelpers::moveEntity(currentPosition, currentMovement,
                                    world.playerInputHistory[tickDiff],
                                    GAME_TICK_TIMESTEP_S);
    }
}

} // namespace Client
} // namespace AM
