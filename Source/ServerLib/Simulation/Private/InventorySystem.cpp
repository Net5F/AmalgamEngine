#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "InventoryInit.h"
#include "SystemMessage.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace Server
{
InventorySystem::InventorySystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, extension{nullptr}
, inventoryAddItemQueue{inNetwork.getEventDispatcher()}
, inventoryDeleteItemQueue{inNetwork.getEventDispatcher()}
, inventoryMoveItemQueue{inNetwork.getEventDispatcher()}
{
    // Observe player Inventory construction events.
    playerInventoryObserver.connect(
        world.registry, entt::collector.group<ClientSimData, Inventory>());
}

void InventorySystem::sendInventoryInits()
{
    // If a player Inventory was constructed, send the initial state to that 
    // player.
    for (entt::entity entity : playerInventoryObserver) {
        auto [client, inventory]
            = world.registry.get<ClientSimData, Inventory>(entity);

        InventoryInit inventoryInit{};
        for (const Inventory::ItemSlot& itemSlot : inventory.items) {
            Uint16 version{world.itemData.getItemVersion(itemSlot.ID)};
            inventoryInit.items.emplace_back(itemSlot.ID, itemSlot.count,
                                             version);
        }

        network.serializeAndSend(client.netID, inventoryInit);
    }
}

void InventorySystem::processInventoryUpdates()
{
    // Process any waiting inventory operations.
    InventoryAddItem inventoryAddItem{};
    while (inventoryAddItemQueue.pop(inventoryAddItem)) {
        addItem(inventoryAddItem);
    }

    InventoryDeleteItem inventoryDeleteItem{};
    while (inventoryDeleteItemQueue.pop(inventoryDeleteItem)) {
        deleteItem(inventoryDeleteItem);
    }

    InventoryMoveItem inventoryMoveItem{};
    while (inventoryMoveItemQueue.pop(inventoryMoveItem)) {
        moveItem(inventoryMoveItem);
    }
}

void InventorySystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void InventorySystem::addItem(const InventoryAddItem& inventoryAddItem)
{
    // If the entity isn't valid, skip it.
    entt::entity entityToAddTo{inventoryAddItem.entity};
    if (!(world.entityIDIsInUse(entityToAddTo))) {
        return;
    }
    // If the project says the request isn't valid, skip it.
    else if ((extension != nullptr)
             && !(extension->isInventoryAddItemValid(inventoryAddItem))) {
        return;
    }

    // Try to add the item.
    bool itemExists{world.itemData.itemExists(inventoryAddItem.itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToAddTo)};
    if (itemExists
        && inventory->addItem(inventoryAddItem.itemID,
                              inventoryAddItem.count)) {
        // Success. If this is a client entity, tell it about the new item.
        if (auto* client{
                world.registry.try_get<ClientSimData>(entityToAddTo)}) {
            network.serializeAndSend(client->netID, inventoryAddItem);
        }
    }
    else {
        network.serializeAndSend(
            inventoryAddItem.netID,
            SystemMessage{"Failed to add item (invalid item ID or "
                          "inventory is full)."});
    }
}

void InventorySystem::deleteItem(const InventoryDeleteItem& inventoryDeleteItem)
{
    // Note: "Delete item" always applies to the client's own inventory 
    //       and they're always allowed to do it, so we don't need a 
    //       project validity check.

    // Find the client's entity ID.
    auto it{world.netIdMap.find(inventoryDeleteItem.netID)};
    if (it != world.netIdMap.end()) {
        entt::entity clientEntity{it->second};

        // If the deletion is successful, tell the client.
        auto* inventory{world.registry.try_get<Inventory>(clientEntity)};
        if (inventory
            && inventory->deleteItem(inventoryDeleteItem.slotIndex,
                                     inventoryDeleteItem.count)) {
            network.serializeAndSend(inventoryDeleteItem.netID,
                                     inventoryDeleteItem);
        }
    }
}

void InventorySystem::moveItem(const InventoryMoveItem& inventoryMoveItem)
{
    // Note: "Move item" always applies to the client's own inventory 
    //       and they're always allowed to do it, so we don't need a 
    //       project validity check.

    // Find the client's entity ID.
    auto it{world.netIdMap.find(inventoryMoveItem.netID)};
    if (it != world.netIdMap.end()) {
        entt::entity clientEntity{it->second};

        // If the move is successful, tell the client.
        auto* inventory{world.registry.try_get<Inventory>(clientEntity)};
        if (inventory
            && inventory->moveItem(inventoryMoveItem.sourceSlotIndex,
                                   inventoryMoveItem.destSlotIndex)) {
            network.serializeAndSend(inventoryMoveItem.netID,
                                     inventoryMoveItem);
        }
    }
}

} // namespace Server
} // namespace AM
