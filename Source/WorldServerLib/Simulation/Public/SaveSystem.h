#pragma once

#include "ItemID.h"
#include "Timer.h"
#include "BinaryBuffer.h"
#include <vector>

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;
class ItemData;

/**
 * Periodically saves the world's data:
 *   Tile map data is saved to TileMap.bin.
 *   Non-client entity data is saved to the database.
 *   Client entity ClientPersistence::World data is saved to the database,
 *   keyed by account ID. ClientPersistence::Account data is queued to be
 *   saved to the account server (see AccountSaveQueue).
 *   Item data is saved to the database.
 *
 * Note: Client entities are also saved when they're removed from the world
 *       (see ClientConnectionSystem).
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
     * Saves items to the in-memory database.
     */
    void saveItems();

    /**
     * Saves World::globalStoredValueMap to the in-memory database.
     */
    void saveGlobalStoredValues();

    World& world;
    ItemData& itemData;

    /** Holds a history of items that have been updated.
        Used to know which items need to be saved. */
    std::vector<ItemID> updatedItems;

    /** Used to track how much time has passed since the last save. */
    Timer saveTimer;

    /** Scratch buffer used while serializing data. */
    BinaryBuffer workBuffer;
};

} // namespace WorldServer
} // namespace AM
