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
}

void NpcMovementSystem::updateNpcs()
{
    // Receive any updates from the server.
    receiveEntityUpdates();

    // We want to process updates until we've either hit the desired, or run out of data.
    Uint32 desiredTick = game.getCurrentTick() - PAST_TICK_OFFSET;

    /* While we have data to use, apply updates for all unprocessed ticks including the
       desired tick. */
    while ((lastProcessedTick <= desiredTick) && (stateUpdateQueue.size() > 0)) {
        // TODO: This should be progressing, even if there's no npc update
        // Move all NPCs as if their inputs didn't change.
        moveAllNpcs();

        // Check that the processed tick is progressing incrementally.
        NpcStateUpdate& stateUpdate = stateUpdateQueue.front();
        if (stateUpdate.tickNum != (lastProcessedTick + 1)) {
            DebugError("Processing NPC movement out of order. stateUpdate.tickNum: %u, "
                       "lastProcessedTick: %u", stateUpdate.tickNum, lastProcessedTick);
        }

        // If the update contained new data, apply it.
        if (stateUpdate.dataChanged) {
            const fb::Message* message = fb::GetMessage(stateUpdate.message->data());
            applyUpdateMessage(message);
        }

        /* Prepare for the next iteration. */
        lastProcessedTick++;
        stateUpdateQueue.pop();
    }
}

void NpcMovementSystem::receiveEntityUpdates()
{
    /* Process any NPC update messages from the Network. */
    BinaryBufferSharedPtr receivedBuffer = network.receive(MessageType::NpcUpdate);
    while (receivedBuffer != nullptr) {
        // Ready the Message for reading.
        const fb::Message* message = fb::GetMessage(receivedBuffer->data());

        // Check that we received the expected tick.
        Uint32 newTick = message->tickTimestamp();
        if (latestReceivedTick == 0) {
            // Received our first message, init our values.
            latestReceivedTick = game.getCurrentTick() - Network::INITIAL_TICK_OFFSET - 1;
            lastProcessedTick = latestReceivedTick;
            DebugInfo("Init lastReceivedTick to: %u", latestReceivedTick);
        }

        // If we received an update message.
        if (newTick != 0) {
            // If there's a gap > 1 between the latest received tick and this update's tick,
            // we know that no changes happened on the in-between ticks and can push
            // confirmations for them.
            unsigned int implicitConfirmations = newTick - (latestReceivedTick + 1);
            for (unsigned int i = 1; i <= implicitConfirmations; ++i) {
                stateUpdateQueue.push({(latestReceivedTick + i), false, nullptr});
                DebugInfo("Push1: %u", (latestReceivedTick + i));
            }

            // Push the update into the buffer.
            stateUpdateQueue.push({newTick, true, receivedBuffer});
            DebugInfo("Push2: %u", newTick);

            latestReceivedTick = newTick;
        }
        else {
            // Received a message confirming that a tick was processed with no update.
            latestReceivedTick++;
            stateUpdateQueue.push({latestReceivedTick, false, nullptr});
            DebugInfo("Push3: %u", latestReceivedTick);
        }

        receivedBuffer = network.receive(MessageType::PlayerUpdate);
    }
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
//        DebugInfo("%d: (%f, %f) -> (%f, %f)", entityID, currentPosition.x,
//            currentPosition.y, newPosition->x(), newPosition->y());
        currentPosition.x = newPosition->x();
        currentPosition.y = newPosition->y();
    }
}

} // namespace Client
} // namespace AM
