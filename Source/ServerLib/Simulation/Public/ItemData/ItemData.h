#pragma once

#include "ItemDataBase.h"

namespace AM
{
namespace Server
{
/**
 * See ItemDataBase class comment.
 *
 * In addition, performs server-specific duties such as saving/loading item
 * definitions.
 */
class ItemData : public ItemDataBase
{
public:
    /**
     * Calls ItemDataBase() constructor and loads our persisted items.
     */
    ItemData();

    /**
     * Creates a new item with the given data.
     * If referenceItem.numericID == NULL_ITEM_ID, uses the next sequential ID.
     *
     * Note: If no stringID is given, it will be derived from referenceItem's 
     *       displayName.
     *
     * @param referenceItem The item to copy when creating the new item.
     * @return If an item with the given ID or displayName exists, does nothing
     *         and returns nullptr. Else, returns the new item.
     */
    const Item* createItem(const Item& referenceItem,
                           std::string_view initScript);

    /**
     * Updates the item at referenceItem.numericID to match the given item, 
     * then increments its version number.
     *
     * Note: If no stringID is given, it will be derived from referenceItem's 
     *       displayName.
     *
     * @param referenceItem The item to copy when creating the new item.
     * @return If no item with the given ID exists or referenceItem's 
     *         displayName is changed but already taken, returns nullptr. 
     *         Else, returns the updated item.
     */
    const Item* updateItem(const Item& referenceItem,
                           std::string_view initScript);

    /**
     * Overload to handle init scripts.
     * Note: Doing it this way means the signals will go out before the init 
     *       script is added to the map. If this becomes an issue, we can 
     *       move it into the base class and just have the client pass "".
     */
    const Item* loadItem(const Item& referenceItem, ItemVersion version,
                         std::string_view initScript);

    /**
     * Returns an item's init script.
     *
     * @return If no item with the given ID exists, returns {}. Else, returns 
     *         the item's init script.
     */
    const ItemInitScript& getItemInitScript(ItemID numericID) const;

private:
    // Note: We bring this into the private namespace so that outside consumers
    //       are forced to use the initScript version.
    using ItemDataBase::loadItem;

    /** Each item's init script, indexed by their numeric IDs.
        Note: We split this from the Item class to reduce size and because it 
              isn't really part of an item's definition. */
    std::unordered_map<ItemID, ItemInitScript> itemInitScriptMap;

    /** Used as a default to return if getItemInitScript() fails. */
    ItemInitScript defaultInitScript;

    /** A scratch buffer used while processing string IDs. */
    std::string workStringID{};
};

} // End namespace Server
} // End namespace AM
