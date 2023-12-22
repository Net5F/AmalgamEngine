#include "InventoryHelpers.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ClientSimData.h"
#include "InventoryOperation.h"
#include "SystemMessage.h"
#include <algorithm>

namespace AM
{
namespace Server
{

bool InventoryHelpers::addItem(ItemID itemID, Uint8 count,
                               entt::entity entityToAddTo, World& world,
                               Network& network,
                               std::optional<NetworkID> requesterID)
{
    // Try to add the item.
    bool itemExists{world.itemData.itemExists(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToAddTo)};
    if (itemExists && inventory && inventory->addItem(itemID, count)) {
        // Success. If the target is a client entity, update it.
        if (auto* client{
                world.registry.try_get<ClientSimData>(entityToAddTo)}) {
            InventoryOperation operation{
                InventoryAddItem{entityToAddTo, itemID, count,
                                 world.itemData.getItemVersion(itemID)}};
            network.serializeAndSend(client->netID, operation);
        }

        return true;
    }
    else if (requesterID) {
        // Failure. If this operation was requested by a client, send it an 
        // error message.
        if (itemExists && inventory) {
            SystemMessage response{"Failed to add item: inventory is full."};
            network.serializeAndSend(requesterID.value(), response);
        }
        else {
            SystemMessage response{
                "Failed to add item: invalid item ID or inventory "
                "doesn't exist."};
            network.serializeAndSend(requesterID.value(), response);
        }
    }

    return false;
}

bool InventoryHelpers::addItem(const std::string& itemID, Uint8 count,
                               entt::entity entityToAddTo, World& world,
                               Network& network,
                               std::optional<NetworkID> requesterID)
{
    if (const Item* item{world.itemData.getItem(itemID)}) {
        return addItem(item->numericID, count, entityToAddTo, world, network,
                       requesterID);
    }
    else if (requesterID) {
        SystemMessage response{"Failed to add item: invalid item ID."};
        network.serializeAndSend(requesterID.value(), response);
    }

    return false;
}

bool InventoryHelpers::removeItem(Uint8 slotIndex, Uint8 count,
                                  entt::entity entityToRemoveFrom, World& world,
                                  Network& network)
{
    // Try to remove the item.
    auto* inventory{world.registry.try_get<Inventory>(entityToRemoveFrom)};
    if (inventory && inventory->removeItem(slotIndex, count)) {
        // Success. If the target is a client entity, update it.
        if (auto* client{
                world.registry.try_get<ClientSimData>(entityToRemoveFrom)}) {
            InventoryOperation operation{InventoryRemoveItem{slotIndex, count}};
            network.serializeAndSend(client->netID, operation);
        }

        return true;
    }

    return false;
}

bool InventoryHelpers::removeItem(const std::string& itemID, Uint8 count,
                                  entt::entity entityToRemoveFrom, World& world,
                                  Network& network)
{
    // If the entity's inventory has enough copies of the item to satisfy the 
    // requested count, remove them.
    const Item* item{world.itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToRemoveFrom)};
    if (item && inventory
        && (inventory->getItemCount(item->numericID) >= count)) {
        auto* client{world.registry.try_get<ClientSimData>(entityToRemoveFrom)};
        Uint8 remainingCount{count};
        for (Uint8 i{0}; i < inventory->slots.size(); ++i) {
            Inventory::ItemSlot& slot{inventory->slots[i]};

            // If this slot contains the item, remove it.
            if (slot.ID == item->numericID) {
                Uint8 amountToRemove{
                    std::clamp<Uint8>(remainingCount, 0, slot.count)};
                inventory->removeItem(i, amountToRemove);

                // If the target is a client entity, send an update.
                if (client) {
                    InventoryOperation operation{
                        InventoryRemoveItem{i, amountToRemove}};
                    network.serializeAndSend(client->netID, operation);
                }

                // If we've found enough copies, stop searching.
                remainingCount -= amountToRemove;
                if (remainingCount == 0) {
                    break;
                }
            }
        }

        return true;
    }

    return false;
}

} // namespace Server
} // namespace AM
