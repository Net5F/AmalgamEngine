#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "ItemData.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "InventoryInit.h"
#include "InventoryHelpers.h"
#include "SystemMessage.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace Server
{
InventorySystem::InventorySystem(World& inWorld, Network& inNetwork,
                                 const ItemData& inItemData)
: world{inWorld}
, network{inNetwork}
, itemData{inItemData}
, extension{nullptr}
, playerInventoryObserver{}
, inventoryOperationQueue{inNetwork.getEventDispatcher()}
{
    // Observe player Inventory construction events.
    playerInventoryObserver.bind(world.registry);
    playerInventoryObserver
        .on_construct<ClientSimData, &EnttObserver::twoComponentCallback>()
        .on_construct<Inventory, &EnttObserver::twoComponentCallback>();
}

void InventorySystem::sendInventoryInits()
{
    // If a player Inventory was constructed, send the initial state to that
    // player.
    for (auto [entity, hasBothComponents] : playerInventoryObserver.each()) {
        if (!hasBothComponents) {
            continue;
        }

        auto [client, inventory]
            = world.registry.get<ClientSimData, Inventory>(entity);

        InventoryInit inventoryInit{inventory.size};
        for (const Inventory::ItemSlot& itemSlot : inventory.slots) {
            ItemVersion version{0};
            if (itemSlot.ID) {
                version = itemData.getItemVersion(itemSlot.ID);
            }
            inventoryInit.slots.emplace_back(itemSlot.ID, itemSlot.count,
                                             version);
        }

        if (inventoryInit.slots.size() > 0) {
            network.serializeAndSend(client.netID, inventoryInit);
        }

        playerInventoryObserver.remove(entity);
    }
}

void InventorySystem::processInventoryUpdates()
{
    // Process any waiting inventory operations.
    InventoryOperation inventoryOperation{};
    while (inventoryOperationQueue.pop(inventoryOperation)) {
        std::visit(
            [&](const auto& operation) {
                processOperation(inventoryOperation.netID, operation);
            },
            inventoryOperation.operation);
    }
}

void InventorySystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void InventorySystem::processOperation(NetworkID clientID,
                                       const InventoryAddItem& inventoryAddItem)
{
    // If the entity isn't valid, skip it.
    entt::entity entityToAddTo{inventoryAddItem.entity};
    if (!(world.registry.valid(entityToAddTo))) {
        return;
    }

    // Try to add the item, sending messages appropriately.
    auto result{InventoryHelpers::addItem(inventoryAddItem.itemID,
                                          inventoryAddItem.count, entityToAddTo,
                                          itemData, world, network)};
    if (result == InventoryHelpers::AddResult::InventoryFull) {
        network.serializeAndSend(
            clientID, SystemMessage{"Failed to add item: Inventory is full."});
    }
    else if (result == InventoryHelpers::AddResult::ItemNotFound) {
        network.serializeAndSend(
            clientID, SystemMessage{"Failed to add item: Item not found."});
    }
}

void InventorySystem::processOperation(
    NetworkID clientID, const InventoryRemoveItem& inventoryRemoveItem)
{
    // Note: "Remove item" always applies to the client's own inventory.

    // Find the client's entity ID.
    auto it{world.netIDMap.find(clientID)};
    if (it != world.netIDMap.end()) {
        entt::entity clientEntity{it->second};

        // Try to remove the item, sending messages appropriately.
        auto result{InventoryHelpers::removeItem(inventoryRemoveItem.slotIndex,
                                                 inventoryRemoveItem.count,
                                                 clientEntity, world, network)};
        if (result == InventoryHelpers::RemoveResult::InvalidSlotIndex) {
            network.serializeAndSend(
                clientID,
                SystemMessage{"Failed to remove item: Invalid slot index."});
        }
        else if (result == InventoryHelpers::RemoveResult::InventoryNotFound) {
            network.serializeAndSend(
                clientID, SystemMessage{"Failed to remove item: Entity has no "
                                        "Inventory component."});
        }
    }
}

void InventorySystem::processOperation(
    NetworkID clientID, const InventoryMoveItem& inventoryMoveItem)
{
    // Note: "Move item" always applies to the client's own inventory.

    // Find the client's entity ID.
    auto it{world.netIDMap.find(clientID)};
    if (it != world.netIDMap.end()) {
        entt::entity clientEntity{it->second};

        // If the move is successful, tell the client.
        Inventory& inventory{world.registry.get<Inventory>(clientEntity)};
        if (inventory.moveItem(inventoryMoveItem.sourceSlotIndex,
                               inventoryMoveItem.destSlotIndex)) {
            network.serializeAndSend(clientID,
                                     InventoryOperation{inventoryMoveItem});
        }
    }
}

} // namespace Server
} // namespace AM
