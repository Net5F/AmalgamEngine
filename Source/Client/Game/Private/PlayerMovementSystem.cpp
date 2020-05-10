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

    while (responseBuffer != nullptr) {
        // Ready the EntityUpdate for reading.
        const fb::Message* message = fb::GetMessage(responseBuffer->data());
        auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());

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
        DebugInfo("%d: (%f, %f) -> (%f, %f)", playerID, currentPosition.x,
            currentPosition.y, receivedPosition->x(), receivedPosition->y());
        currentPosition.x = receivedPosition->x();
        currentPosition.y = receivedPosition->y();

        // TODO: Do this stuff after adding player movement processing and npc stuff and testing that
        //       we're back where we were.
        /* Replay inputs newer than the message's tick. */
        Uint32 currentTick = game.getCurrentTick();
        // TODO: Reverse iterate through CircularBuffer, calc every input newer than the received.

        responseBuffer = network.receive(MessageType::PlayerUpdate);
    } // End while

    /* Use the current input state to update movement for this tick. */
    MovementHelpers::moveEntity(currentPosition, currentMovement,
        world.inputs[playerID].inputStates, deltaSeconds);
}

} // namespace Client
} // namespace AM
