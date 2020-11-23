#pragma once

#include "MovementHelpers.h"
#include "GameDefs.h"
#include "InputComponent.h"
#include <array>
#include <vector>
#include <memory>

namespace AM
{
class EntityUpdate;

namespace Client
{
class Game;
class World;
class Network;

/**
 * Processes player entity update messages and moves the entity appropriately.
 */
class PlayerMovementSystem
{
public:
    PlayerMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Moves the player entity 1 sim tick into the future.
     * Receives state messages, moves the player, replays inputs.
     */
    void processMovements();

private:
    /**
     * Receives any player entity updates from the server.
     * @return The tick number of the latest update message that we received.
     */
    Uint32 processReceivedUpdates();

    /** Pushes a message confirming that a tick was processed with no update. */
    void handleExplicitConfirmation();
    /** Pushes messages confirming ticks up to confirmedTick. */
    void handleImplicitConfirmation(const Uint32 confirmedTick);
    /** Handles an update message, including implicit confirmations based on it.
     */
    void handleUpdate(const std::shared_ptr<const EntityUpdate>& entityUpdate);

    /**
     * Sets lastConfirmedTick to confirmedTick, then it against the history of
     * sent messages to see if any were dropped.
     * If a message was found to be dropped, rolls back and replaces that input
     * with the previous tick's.
     */
    void processTickConfirmation(Uint32 confirmedTick);

    /**
     * Replay any inputs that are from newer ticks than the lastReceivedUpdate.
     */
    void replayInputs(Uint32 lastReceivedUpdate);

    /**
     * The latest tick that has been confirmed by the server.
     * Includes implicit/explicit confirmations, and updates.
     */
    Uint32 lastConfirmedTick;

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM
