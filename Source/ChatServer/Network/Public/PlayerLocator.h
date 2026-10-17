#pragma once

#include "Vector3.h"
#include <SDL3/SDL_stdinc.h>
#include <unordered_map>

namespace AM
{
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

private:
    /** Maps account IDs -> the position of that account's client entity.
        Only contains players that are in the world. */
    std::unordered_map<Sint64, Vector3> playerPositions;
};

} // End namespace ChatServer
} // End namespace AM
