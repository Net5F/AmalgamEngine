#include "ItemSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "ItemHandlers.h"
#include "ItemUpdate.h"
#include "CombineItems.h"
#include "SystemMessage.h"
#include "Log.h"

namespace AM
{
namespace Server
{
ItemSystem::ItemSystem(Simulation& inSimulation, Network& inNetwork)
: simulation{inSimulation}
, world{inSimulation.getWorld()}
, network{inNetwork}
, extension{nullptr}
, combineItemsRequestQueue{inNetwork.getEventDispatcher()}
, useItemOnEntityRequestQueue{inNetwork.getEventDispatcher()}
, itemUpdateRequestQueue{inNetwork.getEventDispatcher()}
, itemChangeRequestQueue{inNetwork.getEventDispatcher()}
{
}

void ItemSystem::processItemInteractions()
{
    // Process any waiting interactions.
    Simulation::ItemInteractionData examineRequest{};
    while (simulation.popItemInteractionRequest(ItemInteractionType::Examine,
                                                examineRequest)) {
        examineItem(examineRequest.item, examineRequest.clientID);
    }

    CombineItemsRequest combineItemsRequest{};
    while (combineItemsRequestQueue.pop(combineItemsRequest)) {
        combineItems(combineItemsRequest.sourceSlotIndex,
                     combineItemsRequest.targetSlotIndex,
                     combineItemsRequest.netID);
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
    ItemData& itemData{world.itemData};

    // Process any waiting requests to create or change items.
    // TODO: If request fails, send a response
    ItemChangeRequest itemChangeRequest{};
    while (itemChangeRequestQueue.pop(itemChangeRequest)) {
        // If the project says the request isn't valid, skip it.
        if (extension
            && !(extension->isItemChangeRequestValid(itemChangeRequest))) {
            continue;
        }

        // If the item exists, update it. Else, create it.
        if (itemData.itemExists(itemChangeRequest.item.numericID)) {
            itemData.updateItem(itemChangeRequest.item);
        }
        else {
            itemData.createItem(itemChangeRequest.item);
        }
    }

    // If any items definitions were changed, send the new definitions to all 
    // players that own that item.
    const auto& updatedItems{itemData.getItemUpdateHistory()};
    if (updatedItems.size() > 0) {
        // If any player's inventory contains an updated item, send the new 
        // definition.
        auto view{world.registry.view<ClientSimData, Inventory>()};
        for (auto [entity, client, inventory] : view.each()) {
            for (const Inventory::ItemSlot& itemSlot : inventory.items) {
                auto it{std::find(updatedItems.begin(), updatedItems.end(),
                                  itemSlot.ID)};
                if (it != updatedItems.end()) {
                    const Item& item{*(itemData.getItem(itemSlot.ID))};
                    network.serializeAndSend(
                        client.netID,
                        ItemUpdate{item.displayName, item.stringID,
                                   item.numericID, item.iconID,
                                   item.supportedInteractions});
                }
            }
        }

        itemData.clearItemUpdateHistory();
    }

    // Send item definitions to any requestors.
    ItemUpdateRequest itemUpdateRequest{};
    while (itemUpdateRequestQueue.pop(itemUpdateRequest)) {
        if (const Item* item{itemData.getItem(itemUpdateRequest.itemID)}) {
            network.serializeAndSend(itemUpdateRequest.netID,
                ItemUpdate{item->displayName, item->stringID, item->numericID,
                           item->iconID, item->supportedInteractions});
        }
    }
}

void ItemSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void ItemSystem::examineItem(const Item* item, NetworkID clientID)
{
    // If the item in the given slot has a description, send it.
    if (const ItemDescription* 
            itemDescription{item->getProperty<ItemDescription>()}) {
        network.serializeAndSend(clientID,
                                 SystemMessage{itemDescription->text});
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
        auto& inventory{world.registry.get<Inventory>(clientEntity)};
        const ItemCombination* combination{inventory.combineItems(
            sourceSlotIndex, targetSlotIndex, world.itemData)};
        if (combination) {
            ItemID resultItemID{combination->resultItemID};
            ItemVersion resultItemVersion{
                world.itemData.getItemVersion(resultItemID)};
            network.serializeAndSend(
                clientID, CombineItems{sourceSlotIndex, targetSlotIndex,
                                       resultItemID, resultItemVersion});
            network.serializeAndSend(clientID,
                                     SystemMessage{combination->description});
        }
    }
}

void ItemSystem::useItemOnEntity(Uint8 sourceSlotIndex,
                                 entt::entity targetEntity, NetworkID clientID)
{
    // Find the client's entity ID.
    auto it{world.netIdMap.find(clientID)};
    if (it != world.netIdMap.end()) {
        entt::entity clientEntity{it->second};

        // If the slot is invalid or empty, do nothing.
        Inventory& inventory{world.registry.get<Inventory>(clientEntity)};
        ItemID sourceItemID{inventory.items[sourceSlotIndex].ID};
        if (!sourceItemID) {
            return;
        }

        // If the target entity has a handler for the item, run it.
        if (const ItemHandlers* itemHandlers{
                world.registry.try_get<ItemHandlers>(targetEntity)}) {
            for (const ItemHandlers::HandlerPair& handlerPair :
                 itemHandlers->handlerPairs) {
                if (handlerPair.itemToHandleID == sourceItemID) {
                    handlerPair.handler();
                }
            }
        }
    }
}

} // namespace Server
} // namespace AM
