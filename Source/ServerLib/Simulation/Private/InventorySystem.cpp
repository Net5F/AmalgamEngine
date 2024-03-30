#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "InventoryInit.h"
#include "InventoryHelpers.h"
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
, playerInventoryObserver{}
, inventoryOperationQueue{inNetwork.getEventDispatcher()}
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

        InventoryInit inventoryInit{inventory.size};
        for (const Inventory::ItemSlot& itemSlot : inventory.slots) {
            ItemVersion version{0};
            if (itemSlot.ID) {
                version = world.itemData.getItemVersion(itemSlot.ID);
            }
            inventoryInit.slots.emplace_back(itemSlot.ID, itemSlot.count,
                                             version);
        }

        if (inventoryInit.slots.size() > 0) {
            network.serializeAndSend(client.netID, inventoryInit);
        }
    }

    playerInventoryObserver.clear();
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
    InventoryHelpers::addItem(inventoryAddItem.itemID, inventoryAddItem.count,
                              entityToAddTo, world, network, clientID);
}

void InventorySystem::processOperation(
    NetworkID clientID, const InventoryRemoveItem& inventoryRemoveItem)
{
    // Note: "Remove item" always applies to the client's own inventory.

    // Find the client's entity ID.
    auto it{world.netIdMap.find(clientID)};
    if (it != world.netIdMap.end()) {
        entt::entity clientEntity{it->second};

        // Try to remove the item, sending messages appropriately.
        InventoryHelpers::removeItem(inventoryRemoveItem.slotIndex,
                                     inventoryRemoveItem.count, clientEntity,
                                     world, network);
    }
}

void InventorySystem::processOperation(
    NetworkID clientID, const InventoryMoveItem& inventoryMoveItem)
{
    // Note: "Move item" always applies to the client's own inventory.

    // Find the client's entity ID.
    auto it{world.netIdMap.find(clientID)};
    if (it != world.netIdMap.end()) {
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
