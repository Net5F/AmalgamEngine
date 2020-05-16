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
    // Check for a message from the server.
    BinaryBufferSharedPtr responseBuffer = network.receive(MessageType::PlayerUpdate);

    EntityID playerID = world.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    // Save the old position.
    world.oldPositions[playerID].x = currentPosition.x;
    world.oldPositions[playerID].y = currentPosition.y;

    /* Receive any player entity updates from the server. */
    // The tick of the newest message that we received.
    Uint32 latestReceivedTick = 0;
    while (responseBuffer != nullptr) {
        // Ready the EntityUpdate for reading.
        const fb::Message* message = fb::GetMessage(responseBuffer->data());
        auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());

        Uint32 newTick = entityUpdate->currentTick();
        if (newTick > latestReceivedTick) {
            latestReceivedTick = newTick;
        }

        // Pull out the vector of entities.
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
    } // End while

    /* Replay inputs newer than the message's tick. */
    Uint32 currentTick = game.getCurrentTick();
    if (latestReceivedTick > currentTick) {
        DebugError("Received data from the future, can't replay inputs. "
                "receivedTick: %u, currentTick %u", latestReceivedTick, currentTick);
    }

    // Only replay inputs if we received a message.
    if (latestReceivedTick != 0) {
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

            /* Use the appropriate input state to update movement. */
            MovementHelpers::moveEntity(currentPosition, currentMovement,
                world.playerInputHistory[tickDiff].inputStates, deltaSeconds);
            DebugInfo("Replayed tick %u", i);
        }
        DebugInfo("%d: Replayed to - (%f, %f) -> (%f, %f)", playerID, recX, recY,
            currentPosition.x, currentPosition.y);
    }

    /* Use the current input state to update movement for this tick. */
    MovementHelpers::moveEntity(currentPosition, currentMovement,
        world.inputs[playerID].inputStates, deltaSeconds);
}

} // namespace Client
} // namespace AM
