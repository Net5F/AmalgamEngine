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
    auto* item{world.itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToAddTo)};
    if (item && inventory
        && inventory->addItem(itemID, count, item->maxStackSize)) {
        // Success. If the target is a client entity, update it.
        if (auto* client{
                world.registry.try_get<ClientSimData>(entityToAddTo)}) {
            InventoryOperation operation{
                InventoryAddItem{entityToAddTo, itemID, count, item->maxStackSize,
                                 world.itemData.getItemVersion(itemID)}};
            network.serializeAndSend(client->netID, operation);
        }

        return true;
    }
    else if (requesterID) {
        // Failure. We were provided a client ID, send it an error message.
        if (item && inventory) {
            network.serializeAndSend(
                requesterID.value(),
                SystemMessage{"Failed to add item: inventory is full."});
        }
        else {
            network.serializeAndSend(
                requesterID.value(),
                SystemMessage{"Failed to add item: invalid item ID or "
                              "inventory doesn't exist."});
        }
    }

    return false;
}

bool InventoryHelpers::addItem(std::string_view itemID, Uint8 count,
                               entt::entity entityToAddTo, World& world,
                               Network& network,
                               std::optional<NetworkID> requesterID)
{
    if (const Item* item{world.itemData.getItem(itemID)}) {
        return addItem(item->numericID, count, entityToAddTo, world, network,
                       requesterID);
    }
    else if (requesterID) {
        network.serializeAndSend(
            requesterID.value(),
            SystemMessage{"Failed to add item: invalid item ID."});
    }

    return false;
}

bool InventoryHelpers::removeItem(Uint8 slotIndex, Uint8 count,
                                  entt::entity entityToRemoveFrom, World& world,
                                  Network& network,
                                  std::optional<NetworkID> requesterID)
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
    else if (requesterID) {
        // Failure. We were provided a client ID, send it an error message.
        if (inventory) {
            network.serializeAndSend(
                requesterID.value(),
                SystemMessage{"Failed to remove item: invalid slot index."});
        }
        else {
            network.serializeAndSend(
                requesterID.value(),
                SystemMessage{
                    "Failed to remove item: inventory doesn't exist."});
        }
    }

    return false;
}

bool InventoryHelpers::removeItem(std::string_view itemID, Uint8 count,
                                  entt::entity entityToRemoveFrom, World& world,
                                  Network& network,
                                  std::optional<NetworkID> requesterID)
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
    else if (requesterID) {
        // Failure. We were provided a client ID, send it an error message.
        if (item && inventory) {
            network.serializeAndSend(
                requesterID.value(),
                SystemMessage{
                    "Failed to remove item: not enough items present."});
        }
        else {
            network.serializeAndSend(
                requesterID.value(),
                SystemMessage{"Failed to remove item: invalid item ID or "
                              "inventory doesn't exist."});
        }
    }

    return false;
}

} // namespace Server
} // namespace AM
