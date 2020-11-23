#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "EntityUpdate.h"
#include "ClientNetworkDefs.h"
#include "PlayerData.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(Game& inGame, World& inWorld,
                                           Network& inNetwork)
: lastConfirmedTick(0)
, game(inGame)
, world(inWorld)
, network(inNetwork)
{
}

void PlayerMovementSystem::processMovements()
{
    EntityID playerID = world.playerData.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    // Save the old position.
    PositionComponent& oldPosition = world.oldPositions[playerID];
    oldPosition.x = currentPosition.x;
    oldPosition.y = currentPosition.y;

    if (!RUN_OFFLINE) {
        // Receive any player entity updates from the server.
        Uint32 lastReceivedUpdate = processReceivedUpdates();

        // If we received messages, replay inputs newer than the latest.
        if (lastReceivedUpdate != 0) {
            replayInputs(lastReceivedUpdate);

            // Check if there was a mismatch between the positions we had and
            // where the server thought we should be.
            if (oldPosition.x != currentPosition.x
                || oldPosition.y != currentPosition.y) {
                LOG_INFO("Predicted position mismatched after replay: (%.6f, "
                         "%.6f) -> (%.6f, %.6f)",
                         oldPosition.x, oldPosition.y, currentPosition.x,
                         currentPosition.y);
                LOG_INFO("lastReceivedUpdate: %u", lastReceivedUpdate);
            }
        }
    }

    // Use the current input state to update movement for this tick.
    MovementHelpers::moveEntity(currentPosition, currentMovement,
                                world.inputs[playerID].inputStates,
                                GAME_TICK_TIMESTEP_S);
}

Uint32 PlayerMovementSystem::processReceivedUpdates()
{
    /* Process any messages for us from the server. */
    Uint32 lastReceivedUpdate = 0;
    EUReceiveResult receiveResult = network.receivePlayerUpdate();
    while (receiveResult.result == NetworkResult::Success) {
        EUMessage& playerUpdateMessage = receiveResult.message;

        // Handle the message appropriately.
        switch (playerUpdateMessage.updateType) {
            case EUType::ExplicitConfirmation:
                // If we've been initialized, process the confirmation.
                if (lastConfirmedTick != 0) {
                    handleExplicitConfirmation();
                }
                break;
            case EUType::ImplicitConfirmation:
                // If we've been initialized, process the confirmation.
                handleImplicitConfirmation(playerUpdateMessage.tickNum);
                break;
            case EUType::StateUpdate:
                LOG_INFO("update: %u", playerUpdateMessage.message->tickNum);
                handleUpdate(playerUpdateMessage.message);

                // Track our latest received tick.
                Uint32 newTick = playerUpdateMessage.message->tickNum;
                if (newTick > lastReceivedUpdate) {
                    lastReceivedUpdate = newTick;
                }
                else {
                    LOG_ERROR("Received ticks out of order. latest: %u, new: %u",
                              lastReceivedUpdate, newTick);
                }
                break;
        }

        receiveResult = network.receivePlayerUpdate();
    }

    return lastReceivedUpdate;
}

void PlayerMovementSystem::handleExplicitConfirmation()
{
    // Increment the confirmed tick and handle any dropped messages.
    processTickConfirmation(lastConfirmedTick + 1);
}

void PlayerMovementSystem::handleImplicitConfirmation(Uint32 confirmedTick)
{
    // If this is the first confirmation we've received, init lastConfirmedTick
    // so things look incrementally increasing.
    if (lastConfirmedTick == 0) {
        lastConfirmedTick = (confirmedTick - 1);
    }

    // Update the confirmed tick and handle any dropped messages.
    processTickConfirmation(confirmedTick);
}

void PlayerMovementSystem::handleUpdate(
const std::shared_ptr<const EntityUpdate>& entityUpdate)
{
    // If this is the first confirmation we've received, init lastConfirmedTick
    // so things look incrementally increasing.
    if (lastConfirmedTick == 0) {
        lastConfirmedTick = (entityUpdate->tickNum - 1);
    }

    // Update the confirmed tick and handle any dropped messages.
    processTickConfirmation(entityUpdate->tickNum);

    // Pull out the vector of entities.
    const std::vector<Entity>& entities = entityUpdate->entities;

    /* Find the player data. */
    EntityID playerID = world.playerData.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

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

    /* Update the velocities. */
    const MovementComponent& newMovement = playerUpdate->movementComponent;
    currentMovement.velX = newMovement.velX;
    currentMovement.velY = newMovement.velY;

    /* Move to the received position. */
    const PositionComponent& receivedPosition
        = playerUpdate->positionComponent;
    currentPosition.x = receivedPosition.x;
    currentPosition.y = receivedPosition.y;
}

void PlayerMovementSystem::processTickConfirmation(Uint32 confirmedTick)
{
    Uint32 currentTick = game.getCurrentTick();
    if (confirmedTick <= lastConfirmedTick) {
        LOG_ERROR("Confirmed ticks are not increasing as expected. "
        "confirmedTick: %u, lastConfirmedTick: %u", confirmedTick, lastConfirmedTick);
    }
    else if (confirmedTick > currentTick) {
        LOG_ERROR("Received data for tick %u on tick %u. Server is in the "
                  "future, can't replay inputs.",
                  confirmedTick, currentTick);
    }

    // Update the confirmed tick, checking all ticks in-between.
    while (lastConfirmedTick < confirmedTick) {
        lastConfirmedTick++;

        // If we have messages waiting to be acked.
        std::queue<Uint32>& synQueue = world.playerData.synQueue;
        if (synQueue.size() > 0) {
            // Check if any messages were acked or found to be dropped.
            Uint32 nextTickToConfirm = synQueue.front();
            if (lastConfirmedTick < nextTickToConfirm) {
                // Fine, nothing needs to be done.
            }
            else if (lastConfirmedTick == nextTickToConfirm) {
                // Sent message is acked, pop it.
                synQueue.pop();
            }
            else {
                // TODO: Why is the next tick after the real drop showing as dropped?
                LOG_INFO("Detected tick %u was dropped while confirming %u",
                    nextTickToConfirm, lastConfirmedTick);
                // Detected a dropped message, overwrite the offending input.
                Uint32 droppedIndex = currentTick - lastConfirmedTick;
                Uint32 previousIndex = droppedIndex + 1;
                if (previousIndex <= (PlayerData::INPUT_HISTORY_LENGTH - 1)) {
                    // Rewrite the dropped input with the input from the previous tick.
                    world.playerData.playerInputHistory[droppedIndex] =
                        world.playerData.playerInputHistory[previousIndex];
                }
                else {
                    LOG_ERROR("Too few items in the player input history. "
                              "Increase the length or reduce lag. droppedIndex: %u, "
                              "historyLength: %u",
                              droppedIndex, PlayerData::INPUT_HISTORY_LENGTH);
                }

                // Replay from the dropped input forward.
                replayInputs(lastConfirmedTick);

                // Pop the dropped tick.
                synQueue.pop();
            }
        }
    }
}

void PlayerMovementSystem::replayInputs(Uint32 lastReceivedUpdate)
{
    EntityID playerID = world.playerData.playerID;
    PositionComponent& currentPosition = world.positions[playerID];
    MovementComponent& currentMovement = world.movements[playerID];

    Uint32 currentTick = game.getCurrentTick();
    if (lastReceivedUpdate > currentTick) {
        LOG_ERROR("Received data for tick %u on tick %u. Server is in the "
                  "future, can't replay inputs.",
                  lastReceivedUpdate, currentTick);
    }

    /* Replay all inputs since the received message, except the current. */
    for (Uint32 tickToProcess = (lastReceivedUpdate + 1);
         tickToProcess < currentTick; ++tickToProcess) {
        Uint32 tickDiff = currentTick - tickToProcess;

        // The history includes the current tick, so we only have LENGTH - 1
        // worth of previous data to use (i.e. it's 0-indexed).
        if (tickDiff > (PlayerData::INPUT_HISTORY_LENGTH - 1)) {
            LOG_ERROR("Too few items in the player input history. "
                      "Increase the length or reduce lag. tickDiff: %u, "
                      "historyLength: %u",
                      tickDiff, PlayerData::INPUT_HISTORY_LENGTH);
        }

        // Use the appropriate input state to update movement.
        MovementHelpers::moveEntity(currentPosition, currentMovement,
                                    world.playerData.playerInputHistory[tickDiff],
                                    GAME_TICK_TIMESTEP_S);
    }
}

} // namespace Client
} // namespace AM
