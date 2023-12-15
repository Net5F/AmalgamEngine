#include "ItemSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "ItemHandlers.h"
#include "ItemUpdate.h"
#include "ItemError.h"
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
, itemInitRequestQueue{inNetwork.getEventDispatcher()}
, itemUpdateRequestQueue{inNetwork.getEventDispatcher()}
, combineItemsRequestQueue{inNetwork.getEventDispatcher()}
, useItemOnEntityRequestQueue{inNetwork.getEventDispatcher()}
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
    // Process any waiting requests to create or change items.
    ItemInitRequest itemInitRequest{};
    while (itemInitRequestQueue.pop(itemInitRequest)) {
        handleInitRequest(itemInitRequest);
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
                    const Item& item{*(world.itemData.getItem(itemSlot.ID))};
                    network.serializeAndSend(
                        client.netID,
                        ItemUpdate{item.displayName, item.stringID,
                                   item.numericID, item.iconID,
                                   item.supportedInteractions});
                }
            }
        }

        world.itemData.clearItemUpdateHistory();
    }

    // Send item definitions to any requestors.
    ItemUpdateRequest itemUpdateRequest{};
    while (itemUpdateRequestQueue.pop(itemUpdateRequest)) {
        handleUpdateRequest(itemUpdateRequest);
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

void ItemSystem::handleInitRequest(const ItemInitRequest& itemInitRequest)
{
    ItemData& itemData{world.itemData};

    // If the project says the request isn't valid, skip it.
    if (extension
        && !(extension->isItemInitRequestValid(itemInitRequest))) {
        network.serializeAndSend(itemInitRequest.netID,
                                 ItemError{itemInitRequest.displayName, "",
                                           itemInitRequest.itemID,
                                           ItemError::Type::PermissionFailure});
        return;
    }

    // Build the new or updated item.
    Item item{};
    item.displayName = itemInitRequest.displayName;
    item.numericID = itemInitRequest.itemID;
    item.iconID = itemInitRequest.iconID;
    item.initScript = itemInitRequest.initScript;

    // Run the init script. If there was an error, tell the user.
    std::string resultString{
        world.runItemInitScript(item, itemInitRequest.initScript)};
    if (!(resultString.empty())) {
        network.serializeAndSend(itemInitRequest.netID,
                                 ItemError{item.displayName, "", item.numericID,
                                           ItemError::Type::InitScriptFailure});
        network.serializeAndSend(itemInitRequest.netID,
                                 SystemMessage{resultString});
        return;
    }

    // If an item with the given ID exists, update it. Else, create it.
    const Item* newItem{nullptr};
    if (itemData.itemExists(item.numericID)) {
        newItem = itemData.updateItem(item);
    }
    else {
        newItem = itemData.createItem(item);
    }

    // Send the requester the new or updated item's definition.
    // Note: If it's an updated item and the requester owns the item, we'll 
    //       end up double-sending them this update, which isn't a big deal.
    if (newItem) {
        network.serializeAndSend(itemInitRequest.netID,
                                 ItemUpdate{newItem->displayName,
                                            newItem->stringID,
                                            newItem->numericID, newItem->iconID,
                                            newItem->supportedInteractions});
    }
}

void ItemSystem::handleUpdateRequest(const ItemUpdateRequest& itemUpdateRequest)
{
    // If the item exists, send an update.
    bool itemWasFound{false};
    std::visit(
        [&](auto& itemID) {
            if (const Item* item{world.itemData.getItem(itemID)}) {
                itemWasFound = true;
                network.serializeAndSend(
                    itemUpdateRequest.netID,
                    ItemUpdate{item->displayName, item->stringID,
                               item->numericID, item->iconID,
                               item->supportedInteractions});
            }
        },
        itemUpdateRequest.itemID);

    // If the requested item doesn't exist, send an error to the client.
    if (!itemWasFound) {
        ItemID numericID{};
        std::string stringID{};
        std::visit(
            [&](auto& itemID) {
                using T = std::decay_t<decltype(itemID)>;
                if constexpr (std::is_same_v<T, ItemID>) {
                    numericID = itemID;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    stringID = itemID;
                }
            },
            itemUpdateRequest.itemID);
        network.serializeAndSend(itemUpdateRequest.netID,
                                 ItemError{"", stringID, numericID,
                                           ItemError::Type::NotFound});
    }
}

} // namespace Server
} // namespace AM
