#include "InventoryHelper.h"
#include "World.h"
#include "Network.h"
#include "ItemData.h"
#include "Inventory.h"
#include "ClientSimData.h"
#include "InventoryOperation.h"
#include "SystemMessage.h"
#include <algorithm>

namespace AM
{
namespace Server
{

InventoryHelper::InventoryHelper(World& inWorld, Network& inNetwork,
                                 const ItemData& inItemData)
: world{inWorld}
, network{inNetwork}
, itemData{inItemData}
{
}

InventoryHelper::AddResult InventoryHelper::addItem(entt::entity entityToAddTo,
                                                    ItemID itemID, Uint8 count,
                                                    bool sendFailureMessage)
{
    // Try to add the item.
    auto* item{itemData.getItem(itemID)};
    auto& inventory{world.registry.get_or_emplace<Inventory>(entityToAddTo)};
    auto* client{world.registry.try_get<ClientSimData>(entityToAddTo)};
    if (item && inventory.addItem(itemID, count, item->maxStackSize)) {
        // Success. If the target is a client entity, update it.
        if (client) {
            InventoryOperation operation{InventoryAddItem{
                entityToAddTo, itemID, count, item->maxStackSize,
                itemData.getItemVersion(itemID)}};
            network.serializeAndSend(client->netID, operation);
        }

        return AddResult::Success;
    }

    // Failure. Return an error code (and send a message, if appropriate).
    AddResult result{};
    const char* message{};
    if (item) {
        result = AddResult::InventoryFull;
        message = "Failed to add item: Inventory is full.";
    }
    else {
        result = AddResult::ItemNotFound;
        message = "Failed to add item: Item not found.";
    }

    if (sendFailureMessage && client) {
        network.serializeAndSend(client->netID, SystemMessage{message});
    }
    return result;
}

InventoryHelper::AddResult InventoryHelper::addItem(entt::entity entityToAddTo,
                                                    std::string_view itemID,
                                                    Uint8 count,
                                                    bool sendFailureMessage)
{
    if (const Item* item{itemData.getItem(itemID)}) {
        return addItem(entityToAddTo, item->numericID, count);
    }

    // Failure. Return an error code (and send a message, if appropriate).
    auto* client{world.registry.try_get<ClientSimData>(entityToAddTo)};
    if (sendFailureMessage && client) {
        network.serializeAndSend(
            client->netID,
            SystemMessage{"Failed to add item: Item not found."});
    }
    return AddResult::ItemNotFound;
}

InventoryHelper::RemoveResult
    InventoryHelper::removeItem(entt::entity entityToRemoveFrom,
                                Uint8 slotIndex, Uint8 count,
                                bool sendFailureMessage)
{
    // Try to remove the item.
    auto* inventory{world.registry.try_get<Inventory>(entityToRemoveFrom)};
    auto* client{world.registry.try_get<ClientSimData>(entityToRemoveFrom)};
    if (inventory && inventory->removeItem(slotIndex, count)) {
        // Success. If the target is a client entity, update it.
        if (client) {
            InventoryOperation operation{InventoryRemoveItem{slotIndex, count}};
            network.serializeAndSend(client->netID, operation);
        }

        return RemoveResult::Success;
    }

    // Failure. Return an error code (and send a message, if appropriate).
    RemoveResult result{};
    const char* message{};
    if (inventory) {
        result = RemoveResult::InvalidSlotIndex;
        message = "Failed to remove item: Invalid slot index.";
    }
    else {
        result = RemoveResult::InventoryNotFound;
        message = "Failed to remove item: Entity has no Inventory component.";
    }

    if (sendFailureMessage && client) {
        network.serializeAndSend(client->netID, SystemMessage{message});
    }
    return result;
}

InventoryHelper::RemoveResult
    InventoryHelper::removeItem(entt::entity entityToRemoveFrom,
                                std::string_view itemID, std::size_t count,
                                bool sendFailureMessage)
{
    // If the entity's inventory has enough copies of the item to satisfy the
    // requested count, remove them.
    const Item* item{itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToRemoveFrom)};
    auto* client{world.registry.try_get<ClientSimData>(entityToRemoveFrom)};
    if (item && inventory
        && (inventory->getItemCount(item->numericID) >= count)) {
        std::size_t remainingCount{count};
        for (Uint8 i{0}; i < inventory->slots.size(); ++i) {
            Inventory::ItemSlot& slot{inventory->slots[i]};

            // If this slot contains the item, remove it.
            if (slot.ID == item->numericID) {
                Uint8 amountToRemove{std::clamp<Uint8>(
                    static_cast<Uint8>(remainingCount), 0, slot.count)};
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

        return RemoveResult::Success;
    }

    // Failure. Return an error code (and send a message, if appropriate).
    RemoveResult result{};
    const char* message{};
    if (item && inventory) {
        result = RemoveResult::InsufficientItemCount;
        message = "Failed to remove item: Insufficient amount.";
    }
    else if (inventory) {
        result = RemoveResult::InventoryNotFound;
        message = "Failed to remove item: Entity has no Inventory component.";
    }
    else {
        result = RemoveResult::ItemNotFound;
        message = "Failed to remove item: Item not found.";
    }

    if (sendFailureMessage && client) {
        network.serializeAndSend(client->netID, SystemMessage{message});
    }
    return result;
}

bool InventoryHelper::hasItem(entt::entity entity,
                              std::string_view itemID) const
{
    const Item* item{itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entity)};
    if (item && inventory && (inventory->getItemCount(item->numericID) >= 1)) {
        return true;
    }
    return false;
}

std::size_t InventoryHelper::getItemCount(entt::entity entity,
                                          std::string_view itemID) const
{
    const Item* item{itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entity)};
    if (item && inventory) {
        return inventory->getItemCount(item->numericID);
    }
    return 0;
}

} // namespace Server
} // namespace AM
