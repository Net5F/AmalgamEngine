#pragma once

#include "Item.h"
#include "IDPool.h"
#include <SDL_stdinc.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace AM
{

/**
 * Base class for Client::ItemData and Server::ItemData.
 * Holds item data.
 * 
 * You can think of the items in this class as "templates". To actually place 
 * them in the world, you must copy one of these templates into something like 
 * an entity's Inventory.
 *
 * Also adds the "Null" item, for use as a default.
 *
 * Note: Once created, items can't be deleted. You can, however, repurpose an 
 *       ID by updating that item's definition.
 */
class ItemDataBase
{
public:
    /**
     * Requests the item data from the database.
     *
     * @param inTrackItemUpdates  If true, item updates will be pushed into 
     *                            itemUpdateHistory.
     */
    ItemDataBase(bool inTrackItemUpdates);

    /**
     * Creates a new item, giving it the next sequential numeric ID.
     * 
     * @return If an item with the given displayName exists, does nothing and 
     *         returns nullptr. Else, returns the new item.
     */
    const Item* createItem(const std::string& displayName);

    /**
     * Updates the item at newItem.numericID to match the given item, then 
     * increments its version number.
     *
     * @return If no item with the given ID exists or newItem's displayName is 
     *         already taken, returns nullptr. Else, returns the updated item.
     *
     * Note: The updated item's stringID will be derived from newItem's 
     *       displayName, regardless of what newItem's stringID is.
     */
    const Item* updateItem(const Item& newItem);

    /**
     * @return If no item with the given ID exists, returns nullptr. Else, 
     *         returns the requested item.
     */
    const Item* getItem(const std::string& stringID) const;

    /**
     * @return If no item with the given ID exists, returns nullptr. Else, 
     *         returns the requested item.
     */
    const Item* getItem(ItemID numericID) const;

    /**
     * @return If no item with the given ID exists, returns false. Else, 
     *         returns true.
     */
    bool itemExists(ItemID numericID) const;

    /**
     * Get an item's version number.
     * Version numbers increase incrementally each time an item's definition 
     * is changed.
     * 
     * @return If no item with the given ID exists, returns 0. Else, returns 
     *         the item's version number.
     */
    ItemVersion getItemVersion(ItemID numericID);

    /**
     * Get a reference to the vector containing all the items.
     */
    const std::vector<Item>& getAllItems() const;

    /**
     * Returns a vector containing all items that have had their definitions 
     * updated since the last time the vector was cleared.
     */
    const std::vector<ItemID>& getItemUpdateHistory();

    /**
     * Clears the item update history vector.
     */
    void clearItemUpdateHistory();

protected:
    /**
     * Derives a string ID from a display name by making it all lowercase and
     * replacing spaces with underscores.
     */
    std::string deriveStringID(const std::string& displayName);

    /** The loaded items, indexed by their numeric IDs. */
    std::vector<Item> items;

    /** Each item's version number, indexed by their numeric IDs. Each time an
        item's definition is changed, its version gets incremented. */
    std::vector<ItemVersion> itemVersions;

    /** A map for easily looking up items by their string ID. */
    std::unordered_map<std::string, Item*> itemStringMap;

    /** If true, all item updates will be pushed into itemUpdateHistory. */
    bool trackItemUpdates;

    /** Holds a history of items that have been updated.
        ItemSystem uses this history to send updates to clients, then
        clears it. */
    std::vector<ItemID> itemUpdateHistory;
};

} // End namespace AM
