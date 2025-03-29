#pragma once

#include "Item.h"
#include "ItemInitScript.h"
#include "HashTools.h"
#include "entt/signal/sigh.hpp"
#include <SDL_stdinc.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>

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
 * it can be accessed through getItem() to get a default item to use.
 *
 * Note: Once created, items can't be deleted. You can, however, repurpose an
 *       ID by updating that item's definition.
 */
class ItemDataBase
{
public:
    /**
     * Requests the item data from the database.
     */
    ItemDataBase();

    virtual ~ItemDataBase() = default;

    /**
     * Creates a new item or updates an existing one. The resulting item will 
     * exactly match referenceItem, with a version matching the given version.
     *
     * Note: Unlike createItem() and updateItem(), this copies referenceItem's 
     *       IDs and version number with no modification. Only use this when 
     *       loading from the database/cache, or when a client is loading a 
     *       server-given item definition.
     * 
     * @param referenceItem The item to copy when creating the new item.
     * @param version The item's current version number.
     * @return The new item.
     */
    const Item* loadItem(const Item& referenceItem, ItemVersion version);

    /**
     * @return If no item with the given ID exists, returns nullptr. Else,
     *         returns the requested item.
     * Note: The null item doesn't technically exist, but we return a useful
     *       default for it.
     */
    const Item* getItem(std::string_view stringID) const;

    /**
     * @return If no item with the given ID exists, returns nullptr. Else,
     *         returns the requested item.
     * Note: The null item doesn't technically exist, but we return a useful
     *       default for it.
     */
    const Item* getItem(ItemID numericID) const;

    /**
     * Returns an item's version number.
     * Version numbers increase incrementally each time an item's definition
     * is changed.
     *
     * @return If no item with the given ID exists, returns 0. Else, returns
     *         the item's version number.
     */
    ItemVersion getItemVersion(ItemID numericID) const;

    /**
     * Returns a reference to the map containing all the items.
     */
    const std::unordered_map<ItemID, Item>& getAllItems() const;

protected:
    // Note: We use unordered_map instead of vector for items/itemVersions
    //       so that we don't have to allocate/copy the whole vector when
    //       a new item is added.
    /** The loaded items, indexed by their numeric IDs. */
    std::unordered_map<ItemID, Item> itemMap;

    /** A map for easily looking up items by their string ID. */
    std::unordered_map<std::string, Item*, string_hash, std::equal_to<>>
        itemStringMap;

    /** Each item's version number, indexed by their numeric IDs. Each time an
        item's definition is changed, its version gets incremented.
        Note: We split this from the Item class because it's often used
              separately (e.g. sending ID + version for Inventory). */
    std::unordered_map<ItemID, ItemVersion> itemVersionMap;

    /** Tracks the next numeric item ID to use (typically 1 greater than the
        highest ID in our maps). */
    ItemID nextItemID{};

    entt::sigh<void(ItemID)> itemCreatedSig;
    entt::sigh<void(ItemID)> itemUpdatedSig;

public:
    /** An item has been created. */
    entt::sink<entt::sigh<void(ItemID)>> itemCreated;

    /** An item has been updated. */
    entt::sink<entt::sigh<void(ItemID)>> itemUpdated;
};

} // End namespace AM
