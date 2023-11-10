#include "ItemSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "SystemMessage.h"
#include "Log.h"

namespace AM
{
namespace Server
{
ItemSystem::ItemSystem(Simulation& inSimulation, Network& inNetwork)
: world{inSimulation.getWorld()}
, network{inNetwork}
, extension{nullptr}
, destroyInteractionQueue{}
, examineInteractionQueue{}
, combineItemsQueue{inNetwork.getEventDispatcher()}
, useItemOnEntityRequestQueue{inNetwork.getEventDispatcher()}
, itemRequestQueue{inNetwork.getEventDispatcher()}
, itemChangeRequestQueue{inNetwork.getEventDispatcher()}
{
    // Subscribe to receive any Examine interactions.
    inSimulation.registerInteractionQueue(ItemInteractionType::Examine,
                                          examineInteractionQueue);
}

void ItemSystem::processItemInteractions()
{
    // Process any waiting interactions.
    while (!(examineInteractionQueue.empty())) {
        const ItemInteractionRequest examineRequest{
            examineInteractionQueue.front()};
        examineItem(examineRequest.targetItemID, examineRequest.netID);

        examineInteractionQueue.pop();
    }

    CombineItems combineItemsMessage{};
    while (combineItemsQueue.pop(combineItemsMessage)) {
        combineItems(combineItemsMessage.sourceSlotIndex,
                     combineItemsMessage.targetSlotIndex,
                     combineItemsMessage.netID);
    }

    UseItemOnEntityRequest useItemOnEntityRequest{};
    while (useItemOnEntityRequestQueue.pop(useItemOnEntityRequest)) {
        useItemOnEntity(useItemOnEntityRequest.sourceSlotIndex,
                        useItemOnEntityRequest.targetEntity,
                        useItemOnEntityRequest.netID);
    }
}

void ItemSystem::processItemUpdates()
{
    // Process any waiting requests to change item definitions.
    ItemChangeRequest itemChangeRequest{};
    while (itemChangeRequestQueue.pop(itemChangeRequest)) {
        // If the project says the request isn't valid, skip it.
        if (extension
            && !(extension->isItemChangeRequestValid(itemChangeRequest))) {
            continue;
        }

        world.itemData.updateItem(itemChangeRequest.item);
    }

    // If any items definitions were changed, send the new definitions to all 
    // players that own that item.
    const auto& updatedItems{world.itemData.getItemUpdateHistory()};
    if (updatedItems.size() > 0) {
        // If any player's inventory contains an updated item, send the new 
        // definition.
        auto view{world.registry.view<ClientSimData, Inventory>()};
        for (auto [entity, client, inventory] : view.each()) {
            for (const Inventory::ItemSlot& itemSlot : inventory.items) {
                auto it{std::find(updatedItems.begin(), updatedItems.end(),
                                  itemSlot.ID)};
                if (it != updatedItems.end()) {
                    network.serializeAndSend(
                        client.netID, *(world.itemData.getItem(itemSlot.ID)));
                }
            }
        }

        world.itemData.clearItemUpdateHistory();
    }

    // Send item definitions to any requestors.
    ItemRequest itemRequest{};
    while (itemRequestQueue.pop(itemRequest)) {
        if (const Item* item{world.itemData.getItem(itemRequest.itemID)}) {
            network.serializeAndSend(itemRequest.netID, *item);
        }
    }
}

void ItemSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void ItemSystem::examineItem(ItemID targetItemID, NetworkID clientID)
{
    // If the given item has a description, send it.
    if (const Item* item{world.itemData.getItem(targetItemID)}) {
        if (const ItemProperty* property{item->getProperty<Description>()}) {
            const Description& description{std::get<Description>(*property)};
            network.serializeAndSend(clientID, SystemMessage{description.text});
        }
    }
}

void ItemSystem::combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
                              NetworkID clientID)
{
    // Find the client's entity ID.
    auto it{world.netIdMap.find(clientID)};
    if (it != world.netIdMap.end()) {
        entt::entity clientEntity{it->second};

        // If the combination is successful, tell the client.
        // Note: All clients have inventories so we don't need to check for it.
        auto& inventory{world.registry.get<Inventory>(it->second)};
        if (inventory.combineItems(sourceSlotIndex, targetSlotIndex,
                                   world.itemData)) {
            network.serializeAndSend(clientID, CombineItems{});
        }
    }
}

void ItemSystem::useItemOnEntity(Uint8 sourceSlotIndex,
                                 entt::entity targetEntity, NetworkID clientID)
{
}

} // namespace Server
} // namespace AM
