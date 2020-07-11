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

void PlayerMovementSystem::processMovements(float deltaSeconds)
{
    // Save the old position.
    EntityID playerID = world.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    world.oldPositions[playerID].x = currentPosition.x;
    world.oldPositions[playerID].y = currentPosition.y;

    if (!Network::RUN_OFFLINE) {
        // Receive any player entity updates from the server.
        Uint32 latestReceivedTick = processReceivedUpdates(playerID, currentPosition,
            currentMovement);

        // If we received messages, replay inputs newer than the latest.
        if (latestReceivedTick != 0) {
            replayInputs(latestReceivedTick, playerID, currentPosition, currentMovement,
                deltaSeconds);
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
    // Check for a message from the server.
    Uint32 latestReceivedTick = 0;
    BinaryBufferSharedPtr responseBuffer = network.receive(MessageType::PlayerUpdate);
    while (responseBuffer != nullptr) {
        // Ready the Message for reading.
        const fb::Message* message = fb::GetMessage(responseBuffer->data());

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
            EntityID entityID = (*entityIt)->id();
            if (entityID == playerID) {
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
        currentMovement.maxVelX = newMovement->maxVelX();
        currentMovement.maxVelY = newMovement->maxVelY();

        /* Move to the received position. */
        auto receivedPosition = receivedData->positionComponent();
        DebugInfo("%d: Jumped to - (%f, %f) -> (%f, %f)", playerID, currentPosition.x,
            currentPosition.y, receivedPosition->x(), receivedPosition->y());
        currentPosition.x = receivedPosition->x();
        currentPosition.y = receivedPosition->y();

        responseBuffer = network.receive(MessageType::PlayerUpdate);
    }

    return latestReceivedTick;
}

void PlayerMovementSystem::replayInputs(Uint32 latestReceivedTick, EntityID playerID,
                                        PositionComponent& currentPosition,
                                        MovementComponent& currentMovement,
                                        float deltaSeconds)
{
    Uint32 currentTick = game.getCurrentTick();
    if (latestReceivedTick > currentTick) {
        DebugError(
            "Received data for tick %u on tick %u. Server is in the future, can't replay "
                "inputs.", latestReceivedTick, currentTick);
    }

    float recX = currentPosition.x;
    float recY = currentPosition.y;
    DebugInfo("Latest: %u, current: %u", latestReceivedTick, currentTick);

    /* Relay all inputs since the received message, except the current. */
    for (Uint32 i = (latestReceivedTick + 1); i < currentTick; ++i) {
        Uint32 tickDiff = currentTick - i;

        if (tickDiff > World::INPUT_HISTORY_LENGTH) {
            DebugError("Too few items in the player input history. "
                       "Increase the length or reduce lag.");
        }

        // Use the appropriate input state to update movement.
        MovementHelpers::moveEntity(currentPosition, currentMovement,
            world.playerInputHistory[tickDiff].inputStates, deltaSeconds);
        DebugInfo("Replayed tick %u", i);
    }
    DebugInfo("%d: Replayed to - (%f, %f) -> (%f, %f)", playerID, recX, recY,
        currentPosition.x, currentPosition.y);
}

} // namespace Client
} // namespace AM
