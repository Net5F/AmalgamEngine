#pragma once

#include "ItemID.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <string_view>

namespace AM
{
namespace Server
{
class World;
class Network;
class ItemData;

/**
 * Helper class for manipulating inventories.
 *
 * When inventories are manipulated, the clients that own them need to be
 * updated. Normally we would observe inventory changes and auto-replicate the
 * whole component, but doing so with a large inventory would be inefficient.
 * Instead, we use these functions to both update the inventory and send the
 * necessary client updates.
 *
 * Note: It may be reasonable to replace this approach with something more
 *       automated, like:
 *         1. Each inventory maintains an operation history. Observe Inventory
              updates, send operations to inventory owner.
 *         2. Add a PreviousInventory component. Observe Inventory updates,
 *            send the diff as InventorySetSlot messages.
 *       Our current approach is more efficient than either of these though, as
 *       it doesn't need to observe anything. The tradeoff is being less
 *       convenient, but we only call these functions in a couple places.
 *       If we ever start commonly calling these functions, consider changing
 *       to one of the other approaches.
 */
class InventoryHelper
{
public:
    enum class AddResult { Success, InventoryFull, ItemNotFound };
    enum class RemoveResult {
        Success,
        InvalidSlotIndex,
        InventoryNotFound,
        ItemNotFound,
        InsufficientItemCount
    };

    InventoryHelper(World& inWorld, Network& inNetwork,
                    const ItemData& inItemData);

    /**
     * Adds the given item to the first available slot in the given entity's
     * inventory.
     *
     * If entityToAddTo is a client entity, sends an update message.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToAddTo (if its a client
     *                           entity) to communicate the failure.
     */
    AddResult addItem(entt::entity entityToAddTo, ItemID itemID, Uint8 count,
                      bool sendFailureMessage = true);

    /**
     * Overload for string IDs.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToAddTo (if its a client
     *                           entity) to communicate the failure.
     */
    AddResult addItem(entt::entity entityToAddTo, std::string_view itemID,
                      Uint8 count, bool sendFailureMessage = true);

    /**
     * Removes the item in the given slot in the given entity's inventory.
     *
     * If entityToRemoveFrom is a client entity, sends an update message.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToRemoveFrom (if its a client
     *                           entity) to communicate the failure.
     */
    RemoveResult removeItem(entt::entity entityToRemoveFrom, Uint8 slotIndex,
                            Uint8 count, bool sendFailureMessage = true);

    /**
     * Removes the given count of items from the entity's inventory.
     * Note: This is different behavior from the above function, which acts on a
     *       specific index.
     *
     * If the inventory doesn't have enough items, does nothing.
     *
     * If entityToRemoveFrom is a client entity, sends an update message.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToRemoveFrom (if its a client
     *                           entity) to communicate the failure.
     */
    RemoveResult removeItem(entt::entity entityToRemoveFrom,
                            std::string_view itemID, std::size_t count,
                            bool sendFailureMessage = true);

    /**
     * Returns true if entity posesses at least 1 of the given item.
     */
    bool hasItem(entt::entity entity, std::string_view itemID) const;

    /**
     * Returns how many of the given item the entity possesses.
     */
    std::size_t getItemCount(entt::entity entity,
                             std::string_view itemID) const;

private:
    World& world;

    Network& network;

    const ItemData& itemData;
};

} // namespace Server
} // namespace AM
