#include "EngineLuaBindings.h"
#include "World.h"
#include "Network.h"
#include "ItemHandlers.h"
#include "Inventory.h"
#include "SystemMessage.h"
#include "ItemProperties.h"
#include "InventoryHelpers.h"
#include "sol/sol.hpp"

namespace AM
{
namespace Server
{

EngineLuaBindings::EngineLuaBindings(sol::state& inEntityInitLua,
                                     sol::state& inEntityItemHandlerLua,
                                     sol::state& inItemInitLua, World& inWorld,
                                     Network& inNetwork)
: entityInitLua{inEntityInitLua}
, entityItemHandlerLua{inEntityItemHandlerLua}
, itemInitLua{inItemInitLua}
, world{inWorld}
, network{inNetwork}
{
}

void EngineLuaBindings::addBindings()
{
    // Entity init
    entityInitLua.set_function("addItemHandler",
                               &EngineLuaBindings::addItemHandler, this);

    // Entity item handler
    entityItemHandlerLua.set_function("addItem", &EngineLuaBindings::addItem,
                                      this);
    entityItemHandlerLua.set_function("removeItem",
                                      &EngineLuaBindings::removeItem, this);
    entityItemHandlerLua.set_function("getItemCount",
                                      &EngineLuaBindings::getItemCount, this);
    entityItemHandlerLua.set_function(
        "sendSystemMessage", &EngineLuaBindings::sendSystemMessage, this);

    // Item init
    itemInitLua.set_function("addDescription",
                             &EngineLuaBindings::addDescription, this);
    itemInitLua.set_function("addCombination",
                             &EngineLuaBindings::addCombination, this);
}

void EngineLuaBindings::addItemHandler(const std::string& itemID,
                                       const std::string& handlerScript)
{
    // If the given item exists, add the given handler.
    if (const Item * item{world.itemData.getItem(itemID)}) {
        entt::entity entity{entityInitLua["selfEntityID"]};
        ItemHandlers& itemHandlers{
            world.registry.get_or_emplace<ItemHandlers>(entity)};

        itemHandlers.add(item->numericID, handlerScript);
    }
}

bool EngineLuaBindings::addItem(const std::string& itemID, Uint8 count)
{
    // Try to add the item, sending messages appropriately.
    // Note: We send errors to the user. It may confuse them, but at least
    //       they'll get some feedback.
    entt::entity clientEntity{entityItemHandlerLua["clientEntityID"]};
    NetworkID clientID{entityItemHandlerLua["clientID"]};
    return InventoryHelpers::addItem(itemID, count, clientEntity, world,
                                     network, clientID);
}

bool EngineLuaBindings::removeItem(const std::string& itemID, Uint8 count)
{
    // Try to remove the item, sending messages appropriately.
    // Note: This will walk the whole inventory, looking for enough copies of
    //       the item to satisfy the given count.
    entt::entity clientEntity{entityItemHandlerLua["clientEntityID"]};
    return InventoryHelpers::removeItem(itemID, count, clientEntity, world,
                                        network);
}

std::size_t EngineLuaBindings::getItemCount(ItemID itemID)
{
    // Try to return the count for the given item.
    entt::entity clientEntity{entityItemHandlerLua["clientEntityID"]};
    if (auto* inventory{world.registry.try_get<Inventory>(clientEntity)}) {
        return inventory->getItemCount(itemID);
    }

    return 0;
}

void EngineLuaBindings::sendSystemMessage(const std::string& message)
{
    NetworkID clientID{entityItemHandlerLua["clientID"]};
    network.serializeAndSend(clientID, SystemMessage{message});
}

void EngineLuaBindings::addDescription(const std::string& description)
{
    // All items support the Examine interaction, so we only need to add the
    // description property.
    Item* item{itemInitLua.get<Item*>("selfItemPtr")};
    item->properties.push_back(ItemDescription{description});
}

void EngineLuaBindings::addCombination(const std::string& otherItemID,
                                       const std::string& resultItemID,
                                       const std::string& description)
{
    // Try to add the given combination.
    const Item* otherItem{world.itemData.getItem(otherItemID)};
    const Item* resultItem{world.itemData.getItem(resultItemID)};
    if (otherItem && resultItem) {
        Item* item{itemInitLua.get<Item*>("selfItemPtr")};
        item->itemCombinations.emplace_back(otherItem->numericID,
                                            resultItem->numericID, description);
    }
    else {
        std::string errorString{
            "Failed to add item combination. Item(s) not found: "};
        if (!otherItem) {
            errorString += "\"" + otherItemID + "\" ";
        }
        if (!resultItem) {
            errorString += "\"" + resultItemID + "\" ";
        }

        throw std::runtime_error{errorString};
    }
}

} // namespace Server
} // namespace AM
