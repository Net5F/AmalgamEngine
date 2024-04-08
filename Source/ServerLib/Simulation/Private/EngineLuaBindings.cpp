#include "EngineLuaBindings.h"
#include "EntityInitLua.h"
#include "EntityItemHandlerLua.h"
#include "ItemInitLua.h"
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

EngineLuaBindings::EngineLuaBindings(
    EntityInitLua& inEntityInitLua,
    EntityItemHandlerLua& inEntityItemHandlerLua, ItemInitLua& inItemInitLua,
    World& inWorld, Network& inNetwork)
: entityInitLua{inEntityInitLua}
, entityItemHandlerLua{inEntityItemHandlerLua}
, itemInitLua{inItemInitLua}
, world{inWorld}
, network{inNetwork}
, dialogueChoiceLua{std::make_unique<sol::state>()}
, currentDialogueTopic{nullptr}
{
}

void EngineLuaBindings::addBindings()
{
    // Entity init
    entityInitLua.luaState.set_function(
        "addItemHandler", &EngineLuaBindings::addItemHandler, this);
    entityInitLua.luaState.set_function("topic", &EngineLuaBindings::topic,
                                        this);

    // Entity item handler
    entityItemHandlerLua.luaState.set_function(
        "addItem", &EngineLuaBindings::addItem, this);
    entityItemHandlerLua.luaState.set_function(
        "removeItem", &EngineLuaBindings::removeItem, this);
    entityItemHandlerLua.luaState.set_function(
        "getItemCount", &EngineLuaBindings::getItemCount, this);
    entityItemHandlerLua.luaState.set_function(
        "sendSystemMessage", &EngineLuaBindings::sendSystemMessage, this);

    // Item init
    itemInitLua.luaState.set_function("setDescription",
                                      &EngineLuaBindings::setDescription, this);
    itemInitLua.luaState.set_function(
        "setMaxStackSize", &EngineLuaBindings::setMaxStackSize, this);
    itemInitLua.luaState.set_function("addCombination",
                                      &EngineLuaBindings::addCombination, this);

    // Dialogue choice
    dialogueChoiceLua->set_function("choice", &EngineLuaBindings::choice, this);
}

void EngineLuaBindings::addItemHandler(std::string_view itemID,
                                       std::string_view handlerScript)
{
    // If the given item exists, add the given handler.
    if (const Item * item{world.itemData.getItem(itemID)}) {
        entt::entity entity{entityInitLua.selfEntity};
        ItemHandlers& itemHandlers{
            world.registry.get_or_emplace<ItemHandlers>(entity)};

        itemHandlers.add(item->numericID, handlerScript);
    }
}

void EngineLuaBindings::topic(std::string_view topicName,
                              std::string_view topicScript,
                              std::string_view choiceScript)
{
    entt::entity entity{entityInitLua.selfEntity};
    Dialogue& dialogue{world.registry.get_or_emplace<Dialogue>(entity)};
    if ((dialogue.topics.size() - 1) == SDL_MAX_UINT8) {
        throw std::runtime_error{
            "Failed to add topic. Max count reached (256)."};
    }

    // Add the new topic to the vector and the map.
    Dialogue::Topic& topic{
        dialogue.topics.emplace_back(std::string{topicScript})};
    Uint8 topicIndex{static_cast<Uint8>(dialogue.topics.size() - 1)};
    dialogue.topicIndices.emplace(topicName, topicIndex);

    // Run the given choice script to fill the choices vector.
    currentDialogueTopic = &topic;
    auto result{
        dialogueChoiceLua->script(choiceScript, &sol::script_pass_on_error)};

    // If the choice script ran successfully, save it.
    std::string returnString{""};
    if (!(result.valid())) {
        // Error occurred while running the script.
        sol::error err = result;
        throw std::runtime_error(err.what());
    }
}

bool EngineLuaBindings::addItem(std::string_view itemID, Uint8 count)
{
    // Try to add the item, sending messages appropriately.
    // Note: We send errors to the user. It may confuse them, but at least
    //       they'll get some feedback.
    return InventoryHelpers::addItem(itemID, count,
                                     entityItemHandlerLua.clientEntity, world,
                                     network, entityItemHandlerLua.clientID);
}

bool EngineLuaBindings::removeItem(std::string_view itemID, Uint8 count)
{
    // Try to remove the item, sending messages appropriately.
    // Note: This will walk the whole inventory, looking for enough copies of
    //       the item to satisfy the given count.
    return InventoryHelpers::removeItem(
        itemID, count, entityItemHandlerLua.clientEntity, world, network);
}

std::size_t EngineLuaBindings::getItemCount(ItemID itemID)
{
    // Try to return the count for the given item.
    if (auto* inventory{world.registry.try_get<Inventory>(
            entityItemHandlerLua.clientEntity)}) {
        return inventory->getItemCount(itemID);
    }

    return 0;
}

void EngineLuaBindings::sendSystemMessage(std::string_view message)
{
    network.serializeAndSend(entityItemHandlerLua.clientID,
                             SystemMessage{std::string{message}});
}

void EngineLuaBindings::setDescription(std::string_view description)
{
    // All items support the Examine interaction already, so we only need to 
    // add the ItemDescription property.
    Item* item{itemInitLua.selfItem};

    // If the item already has a description, overwrite it.
    for (ItemProperty& itemProperty : item->properties) {
        if (auto* itemDescription{
                std::get_if<ItemDescription>(&itemProperty)}) {
            itemDescription->text = description;
            return;
        }
    }

    // The item doesn't already have a description. Add one.
    item->properties.push_back(ItemDescription{std::string{description}});
}

void EngineLuaBindings::setMaxStackSize(Uint8 newMaxStackSize)
{
    itemInitLua.selfItem->maxStackSize = newMaxStackSize;
}

void EngineLuaBindings::addCombination(std::string_view otherItemID,
                                       std::string_view resultItemID,
                                       std::string_view description)
{
    // Try to add the given combination.
    const Item* otherItem{world.itemData.getItem(otherItemID)};
    const Item* resultItem{world.itemData.getItem(resultItemID)};
    if (otherItem && resultItem) {
        Item* item{itemInitLua.selfItem};
        item->itemCombinations.emplace_back(otherItem->numericID,
                                            resultItem->numericID,
                                            std::string{description});
    }
    else {
        std::string errorString{
            "Failed to add item combination. Item(s) not found: "};
        if (!otherItem) {
            errorString += "\"" + std::string{otherItemID} + "\" ";
        }
        if (!resultItem) {
            errorString += "\"" + std::string{resultItemID} + "\" ";
        }

        throw std::runtime_error{errorString};
    }
}

void EngineLuaBindings::choice(std::string_view conditionScript,
                               std::string_view displayText,
                               std::string_view actionScript)
{
    currentDialogueTopic->choices.emplace_back(conditionScript, displayText,
                                               actionScript);
}

} // namespace Server
} // namespace AM
