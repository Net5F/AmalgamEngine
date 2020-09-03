#include "NpcMovementSystem.h"
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

NpcMovementSystem::NpcMovementSystem(Game& inGame, World& inWorld, Network& inNetwork)
: latestReceivedTick(0),
  lastProcessedTick(0),
  game(inGame),
  world(inWorld),
  network(inNetwork)
{
    if (static_cast<unsigned int>(PAST_TICK_OFFSET * -1) > UPDATE_MESSAGE_BUFFER_LENGTH) {
        DebugError("Asking for ticks farther in the past than the buffer can hold."
                   "PAST_TICK_OFFSET: %u, UPDATE_MESSAGE_BUFFER_LENGTH: %u"
                   , PAST_TICK_OFFSET, UPDATE_MESSAGE_BUFFER_LENGTH);
    }
}

void NpcMovementSystem::updateNpcs()
{
    // Receive any updates from the server.
    unsigned int messagesReceived = receiveEntityUpdates();
    if (messagesReceived > UPDATE_MESSAGE_BUFFER_LENGTH) {
        DebugError("Received too many NPC update messages at once to fit in the buffer."
                   " messagesReceived: %u", messagesReceived);
    }

    /* Move all NPCs appropriately. */
    // Check if we've received an update for the desired tick.
    Uint32 currentTick = game.getCurrentTick();
    Uint32 desiredTick = currentTick - PAST_TICK_OFFSET;
    if (latestReceivedTick < desiredTick) {
        // No data to process yet.
        return;
    }

    // We may have to process more than one tick (might've missed some if there was lag.)
    unsigned int ticksLeft = desiredTick - lastProcessedTick;

    // TODO: Fix it so that it processes unprocessed ticks, even if we haven't caught back
    //       up to the desired yet.

    // Check that all the desired data is still in the buffer.
    Uint32 indexOfDesiredTick = latestReceivedTick - desiredTick;
    if ((indexOfDesiredTick + ticksLeft) > UPDATE_MESSAGE_BUFFER_LENGTH) {
        DebugError("Received too many NPC update messages at once, unprocessed data was"
                   " pushed out of the buffer. indexOfDesiredTick: %u, ticksLeft: %u"
                   , indexOfDesiredTick, ticksLeft);
    }

    /* We have data to use, apply updates for all unprocessed ticks including the
       desired tick. */
    while (ticksLeft > 0) {
        Uint32 tickToProcess = indexOfDesiredTick + ticksLeft;

        // Apply the update.
        BinaryBufferSharedPtr messageBuffer = updateBuffer[tickToProcess];
        applyUpdateMessage(messageBuffer);

        ticksLeft--;
    }
}

unsigned int NpcMovementSystem::receiveEntityUpdates()
{
    /* Process any messages for us from the server. */
    unsigned int messagesReceived = 0;
    BinaryBufferSharedPtr receivedBuffer = network.receive(MessageType::NpcUpdate);
    while (receivedBuffer != nullptr) {
        // Ready the Message for reading.
        const fb::Message* message = fb::GetMessage(receivedBuffer->data());

        // Check that the we received the expected tick.
        Uint32 newTick = message->tickTimestamp();
        if (newTick != (latestReceivedTick + 1)) {
            DebugError("Didn't receive incrementally increasing ticks. latestReceivedTick:"
                       " %u, newTick: %u", latestReceivedTick, newTick)
        }

        // Push the message into the buffer.
        updateBuffer.push(receivedBuffer);
        latestReceivedTick = newTick;

        messagesReceived++;
        receivedBuffer = network.receive(MessageType::PlayerUpdate);
    }

    return messagesReceived;
}

void NpcMovementSystem::applyUpdateMessage(const BinaryBufferSharedPtr& messageBuffer)
{
    // Pull out the vector of entities.
    const fb::Message* message = fb::GetMessage(messageBuffer->data());
    auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());
    auto entities = entityUpdate->entities();

    // Iterate through the entities, updating all local data.
    for (auto entityIt = entities->begin(); entityIt != entities->end(); ++entityIt) {
        // Skip the non-NPC.
        EntityID entityID = (*entityIt)->id();
        if (entityID == world.playerID) {
            continue;
        }

        // If the entity doesn't exist, add it to our list.
        if (!(world.entityExists(entityID))) {
            DebugInfo("New entity added. ID: %u", entityID);
            world.addEntity((*entityIt)->name()->str(), entityID);

            // TODO: Get this info from the server.
            // Get the same texture as the player.
            world.sprites[entityID].texturePtr =
                world.sprites[world.playerID].texturePtr;
            world.sprites[entityID].posInTexture =
                world.sprites[world.playerID].posInTexture;
            world.sprites[entityID].width = 64;
            world.sprites[entityID].height = 64;

            world.attachComponent(entityID, ComponentFlag::Input);
            world.attachComponent(entityID, ComponentFlag::Movement);
            world.attachComponent(entityID, ComponentFlag::Position);
            world.attachComponent(entityID, ComponentFlag::Sprite);
        }

        /* Update the inputs. */
        std::array<Input::State, Input::NumTypes>& entityInputStates =
            world.inputs[entityID].inputStates;
        auto clientInputStates = (*entityIt)->inputComponent()->inputStates();
        for (unsigned int i = 0; i < Input::NumTypes; ++i) {
            entityInputStates[i] = MessageUtil::convertToAMInputState(
                clientInputStates->Get(i));
        }

        /* Update the movements. */
        MovementComponent& movement = world.movements[entityID];
        auto newMovement = (*entityIt)->movementComponent();
        movement.velX = newMovement->velX();
        movement.velY = newMovement->velY();
        movement.maxVelX = newMovement->maxVelX();
        movement.maxVelY = newMovement->maxVelY();

        /* Save the old position. */
        PositionComponent& oldPosition = world.oldPositions[entityID];
        PositionComponent& currentPosition = world.positions[entityID];
        oldPosition.x = currentPosition.x;
        oldPosition.y = currentPosition.y;

        /* Update the currentPosition. */
        auto newPosition = (*entityIt)->positionComponent();
        DebugInfo("%d: (%f, %f) -> (%f, %f)", entityID, currentPosition.x,
            currentPosition.y, newPosition->x(), newPosition->y());
        currentPosition.x = newPosition->x();
        currentPosition.y = newPosition->y();
    }
}

} // namespace Client
} // namespace AM
