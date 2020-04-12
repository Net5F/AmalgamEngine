#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <Tile.h>
#include <NWStructs.h>
#include <memory>
#include <vector>
#include <string>

namespace NW
{

/**********************************/
/* Forward declarations
/**********************************/
class GameManager;

/**
 * \class TileManager
 * 
 * \brief Builds and maintains tiles and the tile map.
 * 
 * This class parses .desc and .map files to build tiles and the tile map.
 *
 * It also manages the lifespan of the objects that it builds.
 * 
 *
 * \author Michael Puskas
 *
 * Created on: Jan 7, 2019
 *
 */
class TileManager
{
private:
    /* A dynamically allocated array representing the immutable tile
       layer of the game world.

       The map is stored in a 1D array, though it represents a 2D map.
       A given 2D coordinate can be indexed by: (row * width) + column */
    std::unique_ptr<Tile*[]> mapPtr;

    /* Map size (in tiles).

       This is parsed from the map file during BuildTileMap.
     */
    RCVec mapSize;

    /* In charge of the lifespan of its Tiles. */
    std::vector<Tile> tileTypes;

    /* Used for communication */
    const GameManager& gameManager;

public:
    /**
     * Parses the tile description file to fill tileTypes.
     *
     * \param[in] FileName  The name of the tile description file to be used,
     *                      including extension.
     */
    void BuildTileTypes(const std::string& FileName);

    /**
     * Parses the map file to construct the TileMap.
     *
     * \param[in] FileName  The name of the tile map file to be used,
     *                      including extension.
     */
    void BuildTileMap(const std::string& FileName);


    /**
     * Returns a pointer to a const pointer to a const Tile.
     */
    const Tile * const * GetTileMap() const;

    const Tile * GetTileType(int Index) const;

    RCVec GetMapSize();


    TileManager(const GameManager& InGameManager);
};

} /* End namespace NW */

#endif /* End TILEMANAGER_H */
