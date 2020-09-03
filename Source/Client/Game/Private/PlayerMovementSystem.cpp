#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

PlayerMovementSystem::PlayerMovementSystem(Game& inGame, World& inWorld,
                                           Network& inNetwork)
: game(inGame), world(inWorld), network(inNetwork)
{
}

void PlayerMovementSystem::processMovements(double deltaSeconds)
{
    EntityID playerID = world.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    // Save the old position.
    PositionComponent& oldPosition = world.oldPositions[playerID];
    oldPosition.x = currentPosition.x;
    oldPosition.y = currentPosition.y;

    if (!Network::RUN_OFFLINE) {
        // Receive any player entity updates from the server.
        Uint32 latestReceivedTick = processReceivedUpdates(playerID, currentPosition,
            currentMovement);

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, currentPosition, currentMovement,
                deltaSeconds);

            // Check if there was a mismatch between the positions we had and where the
            // server thought we should be.
            if (oldPosition.x != currentPosition.x
            || oldPosition.y != currentPosition.y) {
                DebugInfo(
                    "Predicted position mismatched after replay: (%.6f, %.6f) -> (%.6f, %.6f)",
                    oldPosition.x, oldPosition.y, currentPosition.x, currentPosition.y);
                DebugInfo("latestReceivedTick: %u", latestReceivedTick);
            }
        }
    }

    // Use the current input state to update movement for this tick.
    MovementHelpers::moveEntity(currentPosition, currentMovement,
        world.inputs[playerID].inputStates, deltaSeconds);
}

Uint32 PlayerMovementSystem::processReceivedUpdates(EntityID playerID,
                                                    PositionComponent& currentPosition,
                                                    MovementComponent& currentMovement)
{
    /* Process any messages for us from the server. */
    Uint32 latestReceivedTick = 0;
    BinaryBufferSharedPtr receivedBuffer = network.receive(MessageType::PlayerUpdate);
    while (receivedBuffer != nullptr) {
        // Ready the Message for reading.
        const fb::Message* message = fb::GetMessage(receivedBuffer->data());

        // Track our latest received tick.
        Uint32 newTick = message->tickTimestamp();
        if (newTick > latestReceivedTick) {
            latestReceivedTick = newTick;
        }

        // Pull out the vector of entities.
        auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());
        auto entities = entityUpdate->entities();

        // Find the player data.
        const fb::Entity* receivedData = nullptr;
        for (auto entityIt = entities->begin(); entityIt != entities->end(); ++entityIt) {
            if ((*entityIt)->id() == playerID) {
                receivedData = *entityIt;
                break;
            }
        }

        if (receivedData == nullptr) {
            DebugError(
                "Failed to find player entity in a message that should have contained one.");
        }

        /* Update the movements. */
        auto newMovement = receivedData->movementComponent();
        currentMovement.velX = newMovement->velX();
        currentMovement.velY = newMovement->velY();
        // TODO: Only send max when it changes.
        currentMovement.maxVelX = newMovement->maxVelX();
        currentMovement.maxVelY = newMovement->maxVelY();

        /* Move to the received position. */
        auto receivedPosition = receivedData->positionComponent();
        currentPosition.x = receivedPosition->x();
        currentPosition.y = receivedPosition->y();

        receivedBuffer = network.receive(MessageType::PlayerUpdate);
    }

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick,
                                        PositionComponent& currentPosition,
                                        MovementComponent& currentMovement,
                                        double deltaSeconds)
{
    Uint32 currentTick = game.getCurrentTick();
    if (latestReceivedTick > currentTick) {
        DebugError(
            "Received data for tick %u on tick %u. Server is in the future, can't replay "
                "inputs.", latestReceivedTick, currentTick);
    }

    /* Replay all inputs since the received message, except the current. */
    for (Uint32 i = (latestReceivedTick + 1); i < currentTick; ++i) {
        Uint32 tickDiff = currentTick - i;

        if (tickDiff > World::INPUT_HISTORY_LENGTH) {
            DebugError("Too few items in the player input history. "
                "Increase the length or reduce lag. tickDiff: %u, historyLength: %u",
                tickDiff, World::INPUT_HISTORY_LENGTH);
        }

        // Use the appropriate input state to update movement.
        MovementHelpers::moveEntity(currentPosition, currentMovement,
            world.playerInputHistory[tickDiff].inputStates, deltaSeconds);
    }
}

} // namespace Client
} // namespace AM
