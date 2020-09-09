#include "NpcMovementSystem.h"
#include "MovementHelpers.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include "Message_generated.h"
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
    if (PAST_TICK_OFFSET > UPDATE_MESSAGE_BUFFER_LENGTH) {
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

    // TODO: Can we simplify this?
    //       Maybe while (messagesLeft > 0 && latestProcessedTick <= desiredTick)

    /* Determine if we have data to use. */
    if (latestReceivedTick == lastProcessedTick) {
        // No data to process yet.
        return;
    }

    // We want to process updates until we've either hit the desired, or run out of data.
    Uint32 currentTick = game.getCurrentTick();
    Uint32 desiredTick = currentTick - PAST_TICK_OFFSET;
    Uint32 endTick = (latestReceivedTick < desiredTick) ? latestReceivedTick : desiredTick;

    // We may have to process more than one tick (might've missed some if there was lag.)
    unsigned int ticksLeft = endTick - lastProcessedTick;

    // Check that all the desired data is still in the buffer.
    Uint32 indexOfEndTick = latestReceivedTick - endTick;
    if ((indexOfEndTick + ticksLeft) > UPDATE_MESSAGE_BUFFER_LENGTH) {
        DebugError("Received too many NPC update messages at once, unprocessed data was"
                   " pushed out of the buffer. latestReceivedTick: %u, lastProcessedTick: %u"
                   ", endTick: %u", latestReceivedTick, lastProcessedTick, endTick);
    }

    /* We have data to use, apply updates for all unprocessed ticks including the
       desired tick. */
    while (ticksLeft > 0) {
        Uint32 indexToProcess = indexOfEndTick + (ticksLeft - 1);

        /* Move all NPCs as if their inputs didn't change. */
        moveAllNpcs();

        /* Correct any NPCs that did change inputs. */
        BinaryBufferSharedPtr messageBuffer = updateBuffer[indexToProcess];
        if (messageBuffer == nullptr) {
            DebugError("Tried to retrieve NPC update message but got nullptr. desiredTick:"
                       " %u, endTick: %u, indexOfEndTick: %u, ticksLeft: %u,"
                       " lastProcessedTick: %u", desiredTick, endTick, indexOfEndTick
                       , ticksLeft, lastProcessedTick);
        }

        // If the message was a real update, apply it.
        const fb::Message* message = fb::GetMessage(messageBuffer->data());
        if (message->tickTimestamp() != 0) {
            applyUpdateMessage(message);
        }

        /* Prepare for the next iteration. */
        updateBuffer[indexToProcess] = nullptr;
        ticksLeft--;
        lastProcessedTick = endTick - ticksLeft;
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

        // Check that we received the expected tick.
        Uint32 newTick = message->tickTimestamp();
        if (latestReceivedTick == 0) {
            // Received our first message, init our values.
            latestReceivedTick = newTick - 1;
            lastProcessedTick = latestReceivedTick;
        }

        // Push the message into the buffer.
        updateBuffer.push(receivedBuffer);

        // Update the latestReceivedTick.
        if (newTick == 0) {
            // Received a message confirming that a tick was processed with no update.
            latestReceivedTick++;
        }
        else {
            if (newTick != (latestReceivedTick + 1)) {
                DebugError("Received ticks aren't progressing incrementally."
                       " latestReceivedTick: %u, newTick: %u", latestReceivedTick, newTick);
            }
            latestReceivedTick = newTick;
        }

        messagesReceived++;
        receivedBuffer = network.receive(MessageType::PlayerUpdate);
    }

    return messagesReceived;
}

void NpcMovementSystem::moveAllNpcs() {
    for (size_t entityID = 0; entityID < MAX_ENTITIES; ++entityID) {
        /* Move all NPCs that have an input, position, and movement component. */
        if (entityID != world.playerID
        && (world.componentFlags[entityID] & ComponentFlag::Input)
        && (world.componentFlags[entityID] & ComponentFlag::Position)
        && (world.componentFlags[entityID] & ComponentFlag::Movement)) {
            // Process their movement.
            MovementHelpers::moveEntity(world.positions[entityID],
                world.movements[entityID], world.inputs[entityID].inputStates,
                GAME_TICK_TIMESTEP_S);
        }
    }
}

void NpcMovementSystem::applyUpdateMessage(const fb::Message* message)
{
    // Pull out the vector of entities.
    auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());
    auto entities = entityUpdate->entities();

    /* Use the data in the message to correct any NPCs that did change inputs. */
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
