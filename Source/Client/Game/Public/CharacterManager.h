#ifndef CHARACTERMANAGER_H
#define CHARACTERMANAGER_H

#include <Character.h>
#include <memory>
#include <vector>

namespace NW
{

/**
 * \class CharacterManager
 * 
 * \brief Owns characters and updates them on request.
 * 
 * This class owns all characters currently in memory, including the
 * player character, AI-based NPCs, and other player's characters.
 * 
 * The source of truth for all character data is the server.
 * This class tries its best to replicate or predict the state
 * of the server, including client-side prediction when appropriate.
 *
 * The local source of truth for character position is their position
 * within this class's character map. This class is in charge of keeping
 * the Character class's internal position up to date.
 *
 *
 * \author Michael Puskas
 *
 * Created on: Feb 17, 2019
 *
 */
class CharacterManager
{
private:
    /* A dynamically allocated array representing the character
       layer of the game world.

       The map is stored in a 1D array, though it represents a 2D map.
       A given 2D coordinate can be indexed by: (row * width) + column */
    std::unique_ptr<Character*[]> mapPtr;

    //TODO: Is map size stored here or pulled at runtime
    //      from tilemanager through gamemanager?

public:

};

} /* End namespace NW */

#endif /* End CHARACTERMANAGER_H */
