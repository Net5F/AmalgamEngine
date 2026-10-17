#pragma once

#include "Position.h"
#include <SDL3/SDL_stdinc.h>
#include <unordered_map>
#include <vector>

namespace AM
{
struct Cylinder;

namespace ChatServer
{

/**
 * Tracks which players are in the world, and where they are.
 *
 * Kept up to date by the world server (see PlayerPositionsUpdate and
 * PlayersLeftWorld). Used to route local chat.
 *
 * Note: The world server resends every position after (re)connecting, so
 *       this should be cleared whenever it connects or disconnects.
 *
 * Note: This is named "player" locator instead of something entity-related,
 *       because the chat server doesn't have a concept of entities (it only
 *       knows accounts). "Player" in this context means "the in-world thing
 *       that the account is associated with".
 */
class PlayerLocator
{
public:
    /**
     * Sets the given player's position, marking them as being in the world.
     */
    void setPosition(Sint64 accountID, const Vector3& position);

    /**
     * Marks the given player as no longer being in the world.
     */
    void remove(Sint64 accountID);

    /**
     * Marks every player as no longer being in the world.
     */
    void clear();

    /**
     * Returns the given player's position, or nullptr if they aren't in the
     * world.
     */
    const Position* getPosition(Sint64 accountID) const;

    /**
     * Returns the account IDs of all players whose positions intersect the
     * given cylinder.
     *
     * Note: This checks every player in the world. If the player count grows
     *       large enough for this to matter, we can add a spatial grid.
     */
    std::vector<Sint64>& getPlayers(const Cylinder& cylinder);

private:
    /** Maps account IDs -> the position of that account's client entity.
        Only contains players that are in the world. */
    std::unordered_map<Sint64, Position> playerPositions;

    /** The vector that we use to return results. */
    std::vector<Sint64> returnVector;
};

} // End namespace ChatServer
} // End namespace AM
