#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <Object.h>
#include <memory>
#include <unordered_map>

namespace NW
{

/**********************************/
/* Forward declarations
/**********************************/
class GameManager;

/**
 * \class ObjectManager
 * 
 * \brief Manages the object list and object map.
 * 
 *
 * \author Michael Puskas
 *
 * Created on: Jan 20, 2019
 *
 */
class ObjectManager
{
private:
    /* A dynamically allocated array representing the object layer of
       the game world.

       The map is stored in a 1D array, though it represents a 2D map.
       A given 2D coordinate can be indexed by: (row * width) + column */
    std::unique_ptr<Object*[]> mapPtr;

    /*
       Map size is parsed from the map file during BuildTileMap.

       Initially set through the constructor, then re-set on Rebuild().
     */
    int mapWidth;
    int mapHeight;

    /* In charge of the lifespan of its Objects. */
    std::unordered_map<int, Object> objectMap;

    /* Used for communication */
    const GameManager& gameManager;

public:
    /**
     * Builds an object from data supplied by the server
     * and adds it to the objectMap.
     *
     * \param[in] InObjectData  The data sent by the server.
     */
    //void AddObject(ObjectData InObjectData);

    void RemoveObject(int InObjectID);


    /**
     * Returns a pointer to a const pointer to a const Object.
     */
    const Object * const * GetObjectMap() const;

    void SetMapSize(int InMapWidth, int InMapHeight);


    ObjectManager(const GameManager& InGameManager
                  , int InMapWidth, int InMapHeight);
};

} /* End namespace NW */

#endif /* End OBJECTMANAGER_H */
