#include "InventoryHelpers.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ClientSimData.h"
#include "InventoryOperation.h"
#include <algorithm>

namespace AM
{
namespace Server
{

InventoryHelpers::AddResult
    InventoryHelpers::addItem(ItemID itemID, Uint8 count,
                              entt::entity entityToAddTo, World& world,
                              Network& network)
{
    // Try to add the item.
    auto* item{world.itemData.getItem(itemID)};
    auto& inventory{world.registry.get_or_emplace<Inventory>(entityToAddTo)};
    if (item && inventory.addItem(itemID, count, item->maxStackSize)) {
        // Success. If the target is a client entity, update it.
        if (auto* client{
                world.registry.try_get<ClientSimData>(entityToAddTo)}) {
            InventoryOperation operation{InventoryAddItem{
                entityToAddTo, itemID, count, item->maxStackSize,
                world.itemData.getItemVersion(itemID)}};
            network.serializeAndSend(client->netID, operation);
        }

        return AddResult::Success;
    }

    // Failure. Return an error code.
    if (item) {
        return AddResult::InventoryFull;
    }
    else {
        return AddResult::ItemNotFound;
    }
}

InventoryHelpers::AddResult
    InventoryHelpers::addItem(std::string_view itemID, Uint8 count,
                              entt::entity entityToAddTo, World& world,
                              Network& network)
{
    if (const Item * item{world.itemData.getItem(itemID)}) {
        return addItem(item->numericID, count, entityToAddTo, world, network);
    }

    return AddResult::ItemNotFound;
}

InventoryHelpers::RemoveResult
    InventoryHelpers::removeItem(Uint8 slotIndex, Uint8 count,
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

        return RemoveResult::Success;
    }

    // Failure. Return an error code.
    if (inventory) {
        return RemoveResult::InvalidSlotIndex;
    }
    else {
        return RemoveResult::InventoryNotFound;
    }
}

InventoryHelpers::RemoveResult
    InventoryHelpers::removeItem(std::string_view itemID, Uint8 count,
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

        return RemoveResult::Success;
    }

    // Failure. Return an error code.
    if (item && inventory) {
        return RemoveResult::InsufficientItemCount;
    }
    else if (inventory) {
        return RemoveResult::InventoryNotFound;
    }
    else {
        return RemoveResult::ItemNotFound;
    }
}

} // namespace Server
} // namespace AM
