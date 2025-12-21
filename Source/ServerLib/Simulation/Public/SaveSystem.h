#pragma once

#include "ItemID.h"
#include "Timer.h"
#include "BinaryBuffer.h"
#include <vector>

namespace AM
{
namespace Server
{
struct SimulationContext;
class Simulation;
class World;
class ItemData;

/**
 * Periodically saves the world's data:
 *   Tile map data is saved to TileMap.bin.
 *   Non-client entity data is saved to the database.
 *   Item data is saved to the database.
 */
class SaveSystem
{
public:
    SaveSystem(const SimulationContext& inSimContext);

    /**
     * If data is due for saving, saves it.
     *
     * Configure through Config::SAVE_PERIOD.
     */
    void saveIfNecessary();

private:
    /**
     * Adds the given item to updatedItems.
     */
    void itemUpdated(ItemID itemID);

    /**
     * Saves non-client entities to the in-memory database.
     */
    void saveNonClientEntities();

    /**
     * Saves items to the in-memory database.
     */
    void saveItems();

    /**
     * Saves World::storedValueIDMap and World::globalStoredValueMap to the 
     * in-memory database.
     */
    void saveStoredValues();

    Simulation& simulation;
    World& world;
    ItemData& itemData;

    /** Holds a history of items that have been updated.
        Used to know which items need to be saved. */
    std::vector<ItemID> updatedItems;

    /** Used to track how much time has passed since the last save. */
    Timer saveTimer;

    /** Scratch buffers used while serializing data. */
    BinaryBuffer workBuffer1;
    BinaryBuffer workBuffer2;
};

} // namespace Server
} // namespace AM
