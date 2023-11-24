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
 * Also adds the "Null" item. The null item can't be overwritten or edited, but
 * it can be accessed through getItem() to get a default icon to use.
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
     * Creates a new item with the given ID.
     * If newItem.numericID == NULL_ITEM_ID, uses the next sequential ID.
     * 
     * Note: The new item's stringID will be derived from newItem's displayName,
     *       regardless of what newItem's stringID is.
     * 
     * @return If an item with the given ID or displayName exists, does nothing 
     *         and returns nullptr. Else, returns the new item.
     */
    const Item* createItem(const Item& newItem);

    /**
     * Updates the item at newItem.numericID to match the given item, then 
     * increments its version number.
     *
     * Note: The updated item's stringID will be derived from newItem's 
     *       displayName, regardless of what newItem's stringID is.
     *
     * @return If no item with the given ID exists or newItem's displayName is 
     *         changed but already taken, returns nullptr. Else, returns the 
     *         updated item.
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
     * Get a reference to the map containing all the items.
     */
    const std::unordered_map<ItemID, Item>& getAllItems() const;

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

    // Note: We use unordered_map instead of vector for items/itemVersions 
    //       so that we don't have to allocate/copy the whole vector when 
    //       a new item is added.
    /** The loaded items, indexed by their numeric IDs. */
    std::unordered_map<ItemID, Item> itemMap;

    /** A map for easily looking up items by their string ID. */
    std::unordered_map<std::string, Item*> itemStringMap;

    /** Each item's version number, indexed by their numeric IDs. Each time an
        item's definition is changed, its version gets incremented.
        Note: We split this from the Item class because it's often used 
              separately (e.g. sending ID + version for Inventory). */
    std::unordered_map<ItemID, ItemVersion> itemVersionMap;

    /** If true, all item updates will be pushed into itemUpdateHistory. */
    bool trackItemUpdates;

    /** Holds a history of items that have been updated.
        ItemSystem uses this history to send updates to clients, then
        clears it. */
    std::vector<ItemID> itemUpdateHistory;

    /** Tracks the next numeric item ID to use (typically 1 greater than the 
        highest ID in our maps). */
    ItemID nextItemID{};
};

} // End namespace AM
