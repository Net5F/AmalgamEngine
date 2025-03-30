#include "ItemSystem.h"
#include "World.h"
#include "Network.h"
#include "ItemData.h"
#include "Database.h"
#include "EntityItemHandlerLua.h"
#include "ISimulationExtension.h"
#include "ClientSimData.h"
#include "Inventory.h"
#include "Castable.h"
#include "ItemHandlers.h"
#include "ItemUpdate.h"
#include "ItemError.h"
#include "CombineItems.h"
#include "StringTools.h"
#include "SystemMessage.h"
#include "Log.h"
#include "sol/sol.hpp"
#include <algorithm>

namespace AM
{
namespace Server
{
ItemSystem::ItemSystem(World& inWorld, Network& inNetwork,
                       ItemData& inItemData,
                       EntityItemHandlerLua& inEntityItemHandlerLua)
: world{inWorld}
, network{inNetwork}
, itemData{inItemData}
, entityItemHandlerLua{inEntityItemHandlerLua}
, extension{nullptr}
, updatedItems{}
, itemInitRequestQueue{inNetwork.getEventDispatcher()}
, itemChangeRequestQueue{inNetwork.getEventDispatcher()}
, combineItemsRequestQueue{inNetwork.getEventDispatcher()}
, useItemOnEntityRequestQueue{inNetwork.getEventDispatcher()}
, itemDataRequestQueue{inNetwork.getEventDispatcher()}
{
    // When an item is updated, add it to updatedItems.
    itemData.itemUpdated.connect<&ItemSystem::itemUpdated>(this);

    // Register a callback for item Examine interactions.
    world.castHelper.setOnItemInteractionCompleted(
        ItemInteractionType::Examine,
        [this](const CastInfo& castInfo) { examineItem(castInfo); });
}

void ItemSystem::processUseItemInteractions()
{
    // Process any waiting messages.
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
    ItemChangeRequest itemChangeRequest{};
    while (itemChangeRequestQueue.pop(itemChangeRequest)) {
        handleChangeRequest(itemChangeRequest);
    }

    // If any items definitions were changed, send the new definitions to all
    // players that own that item.
    if (updatedItems.size() > 0) {
        // Remove duplicates from the vector.
        std::sort(updatedItems.begin(), updatedItems.end());
        updatedItems.erase(
            std::unique(updatedItems.begin(), updatedItems.end()),
            updatedItems.end());

        // TODO: Instead of iterating each inventory slot, find a way to 
        //       sort and efficiently search for matches.
        // If any player's inventory contains an updated item, send the new
        // definition.
        auto view{world.registry.view<ClientSimData, Inventory>()};
        for (auto [entity, client, inventory] : view.each()) {
            for (const Inventory::ItemSlot& itemSlot : inventory.slots) {
                auto it{std::find(updatedItems.begin(), updatedItems.end(),
                                  itemSlot.ID)};
                if (it != updatedItems.end()) {
                    const Item& item{*(itemData.getItem(itemSlot.ID))};
                    network.serializeAndSend(
                        client.netID,
                        ItemUpdate{
                            item.displayName, item.numericID, item.iconID,
                            item.maxStackSize, item.supportedInteractions,
                            itemData.getItemVersion(item.numericID)});
                }
            }
        }

        updatedItems.clear();
    }

    // Send item definitions to any requestors.
    ItemDataRequest itemDataRequest{};
    while (itemDataRequestQueue.pop(itemDataRequest)) {
        handleDataRequest(itemDataRequest);
    }
}

void ItemSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void ItemSystem::itemUpdated(ItemID itemID)
{
    updatedItems.emplace_back(itemID);
}

void ItemSystem::examineItem(const CastInfo& castInfo)
{
    // Send the item's description.
    // Note: Since all items have a description, you could imagine sending it  
    //       with the initial ItemUpdate. To save data, we send it when 
    //       requested instead (we assume that people are rarely going to 
    //       examine items, compared to how often we send ItemUpdates).
    network.serializeAndSend(castInfo.clientID,
                             SystemMessage{castInfo.item->description});
}

void ItemSystem::combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
                              NetworkID clientID)
{
    // Find the client's entity ID.
    auto it{world.netIDMap.find(clientID)};
    if (it != world.netIDMap.end()) {
        entt::entity clientEntity{it->second};

        // If the combination is successful, tell the client.
        auto& inventory{world.registry.get<Inventory>(clientEntity)};
        const ItemCombination* combination{inventory.combineItems(
            sourceSlotIndex, targetSlotIndex, itemData)};
        if (combination) {
            ItemID resultItemID{combination->resultItemID};
            const Item* item{itemData.getItem(resultItemID)};
            ItemVersion resultItemVersion{
                itemData.getItemVersion(resultItemID)};
            network.serializeAndSend(
                clientID,
                CombineItems{sourceSlotIndex, targetSlotIndex, resultItemID,
                             item->maxStackSize, resultItemVersion});
            network.serializeAndSend(clientID,
                                     SystemMessage{combination->description});
        }
        else {
            // No combination for the item. Give the user feedback.
            network.serializeAndSend(clientID,
                                     SystemMessage{"Nothing happens."});
        }
    }
}

void ItemSystem::useItemOnEntity(Uint8 sourceSlotIndex,
                                 entt::entity targetEntity, NetworkID clientID)
{
    // Find the client's entity ID.
    auto it{world.netIDMap.find(clientID)};
    if (it != world.netIDMap.end()) {
        entt::entity clientEntity{it->second};

        // If the slot is invalid or empty, do nothing.
        Inventory& inventory{world.registry.get<Inventory>(clientEntity)};
        ItemID sourceItemID{inventory.slots[sourceSlotIndex].ID};
        if (!sourceItemID) {
            return;
        }

        // Try to find a handler for the item.
        const EntityItemHandlerScript* handlerScript{nullptr};
        if (const ItemHandlers* itemHandlers{
                world.registry.try_get<ItemHandlers>(targetEntity)}) {
            for (const ItemHandlers::HandlerPair& handlerPair :
                 itemHandlers->handlerPairs) {
                if (handlerPair.itemToHandleID == sourceItemID) {
                    handlerScript = &(handlerPair.handlerScript);
                    break;
                }
            }
        }

        // If we found a handler, run it.
        if (handlerScript) {
            runEntityItemHandlerScript(clientID, clientEntity, targetEntity,
                                       *handlerScript);
        }
        else {
            // No handler for the item. Give the user feedback.
            network.serializeAndSend(clientID,
                                     SystemMessage{"Nothing happens."});
        }
    }
}

void ItemSystem::handleInitRequest(const ItemInitRequest& itemInitRequest)
{
    // If the string ID is already in use, send an error.
    if (itemData.getItem(itemInitRequest.displayName)) {
        std::string stringID{};
        StringTools::deriveStringID(itemInitRequest.displayName, stringID);
        network.serializeAndSend(itemInitRequest.netID,
                                 ItemError{itemInitRequest.displayName,
                                           stringID, NULL_ITEM_ID,
                                           ItemError::StringIDInUse});
        return;
    }

    // Build the new item.
    Item item{};
    item.displayName = itemInitRequest.displayName;
    item.iconID = itemInitRequest.iconID;

    // Run the init script. If there was an error, return early.
    if (!(runItemInitScript(itemInitRequest.netID, itemInitRequest.initScript,
                            item))) {
        return;
    }

    // Create the item (should always succeed since we checked the string ID).
    const Item* newItem{
        itemData.createItem(item, itemInitRequest.initScript.script)};
    AM_ASSERT(newItem != nullptr, "Failed to create item.");

    // Send the requester the new item's definition.
    network.serializeAndSend(
        itemInitRequest.netID,
        ItemUpdate{newItem->displayName, newItem->numericID, newItem->iconID,
                   newItem->maxStackSize, newItem->supportedInteractions,
                   itemData.getItemVersion(newItem->numericID)});
}

void ItemSystem::handleChangeRequest(const ItemChangeRequest& itemChangeRequest)
{
    // Check that the numeric ID exists.
    ItemError::Type errorType{ItemError::NotSet};
    if (!(itemData.getItem(itemChangeRequest.itemID))) {
        errorType = ItemError::NumericIDNotFound;
    }
    // Check that the string ID isn't taken by another item.
    else if (const Item*
                 item{itemData.getItem(itemChangeRequest.displayName)};
             item && (item->numericID != itemChangeRequest.itemID)) {
        errorType = ItemError::StringIDInUse;
    }

    // If we found an error, send it to the requesting client.
    if (errorType != ItemError::NotSet) {
        std::string stringID{};
        StringTools::deriveStringID(itemChangeRequest.displayName, stringID);
        network.serializeAndSend(itemChangeRequest.netID,
                                 ItemError{itemChangeRequest.displayName,
                                           stringID, itemChangeRequest.itemID,
                                           errorType});
        return;
    }

    // Build the updated item.
    Item item{};
    item.displayName = itemChangeRequest.displayName;
    item.numericID = itemChangeRequest.itemID;
    item.iconID = itemChangeRequest.iconID;

    // Run the init script. If there was an error, return early.
    if (!(runItemInitScript(itemChangeRequest.netID,
                            itemChangeRequest.initScript, item))) {
        return;
    }

    // Update the item (should always succeed since we checked the ID).
    const Item* updatedItem{
        itemData.updateItem(item, itemChangeRequest.initScript.script)};
    AM_ASSERT(updatedItem != nullptr, "Failed to update item.");

    // Send the requester the new item's definition.
    // Note: If the requester owns the item, we'll end up double-sending
    //       them this update, which isn't a big deal.
    network.serializeAndSend(
        itemChangeRequest.netID,
        ItemUpdate{updatedItem->displayName, updatedItem->numericID,
                   updatedItem->iconID, updatedItem->maxStackSize,
                   updatedItem->supportedInteractions,
                   itemData.getItemVersion(updatedItem->numericID)});
}

void ItemSystem::handleDataRequest(const ItemDataRequest& itemDataRequest)
{
    // If the item exists, send an update.
    bool itemWasFound{false};
    std::visit(
        [&](auto& itemID) {
            if (const Item * item{itemData.getItem(itemID)}) {
                itemWasFound = true;
                network.serializeAndSend(
                    itemDataRequest.netID,
                    ItemUpdate{item->displayName, item->numericID, item->iconID,
                               item->maxStackSize, item->supportedInteractions,
                               itemData.getItemVersion(item->numericID)});
            }
        },
        itemDataRequest.itemID);

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
            itemDataRequest.itemID);
        network.serializeAndSend(
            itemDataRequest.netID,
            ItemError{"", stringID, numericID, ItemError::StringIDNotFound});
    }
}

bool ItemSystem::runItemInitScript(NetworkID clientID,
                                   const ItemInitScript& initScript, Item& item)
{
    // Run the given init script.
    std::string resultString{world.runItemInitScript(item, initScript.script)};

    // If there was an error while running the script, tell the user and return
    // false.
    if (!(resultString.empty())) {
        network.serializeAndSend(clientID,
                                 ItemError{item.displayName, "", item.numericID,
                                           ItemError::InitScriptFailure});
        network.serializeAndSend(clientID, SystemMessage{resultString});
        return false;
    }

    return true;
}

void ItemSystem::runEntityItemHandlerScript(
    NetworkID clientID, entt::entity clientEntity, entt::entity targetEntity,
    const EntityItemHandlerScript& itemHandlerScript)
{
    // Run the given handler script.
    entityItemHandlerLua.clientID = clientID;
    entityItemHandlerLua.luaState["user"] = clientEntity;
    entityItemHandlerLua.luaState["self"] = targetEntity;
    auto result{entityItemHandlerLua.luaState.script(
        itemHandlerScript.script, &sol::script_pass_on_error)};

    // If there was an error while running the handler script, tell the
    // user.
    if (!(result.valid())) {
        sol::error err = result;
        network.serializeAndSend(clientID, SystemMessage{err.what()});
    }
}

} // namespace Server
} // namespace AM
