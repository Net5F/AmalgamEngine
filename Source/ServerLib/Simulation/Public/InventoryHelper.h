#pragma once

#include "ItemID.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
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
    AddResult addItemToEntity(ItemID itemID, Uint8 count,
                              entt::entity entityToAddTo,
                              bool sendFailureMessage = true);

    /**
     * Overload for string IDs.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToAddTo (if its a client 
     *                           entity) to communicate the failure.
     */
    AddResult addItemToEntity(std::string_view itemID, Uint8 count,
                              entt::entity entityToAddTo,
                              bool sendFailureMessage = true);

    /**
     * Removes the item in the given slot in the given entity's inventory.
     *
     * If entityToAddTo is a client entity, sends an update message.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToRemoveFrom (if its a client 
     *                           entity) to communicate the failure.
     */
    RemoveResult removeItemFromEntity(Uint8 slotIndex, Uint8 count,
                                      entt::entity entityToRemoveFrom,
                                      bool sendFailureMessage = true);

    /**
     * Removes the given count of items from the entity's inventory.
     * Note: This is different behavior from the above function, which acts on a
     *       specific index.
     *
     * If the inventory doesn't have enough items, does nothing.
     *
     * If entityToAddTo is a client entity, sends an update message.
     *
     * @param sendFailureMessage If true and result != Success, a message will
     *                           be sent to entityToRemoveFrom (if its a client 
     *                           entity) to communicate the failure.
     */
    RemoveResult removeItemFromEntity(std::string_view itemID, Uint8 count,
                                      entt::entity entityToRemoveFrom,
                                      bool sendFailureMessage = true);

private:
    World& world;

    Network& network;

    const ItemData& itemData;
};

} // namespace Server
} // namespace AM
