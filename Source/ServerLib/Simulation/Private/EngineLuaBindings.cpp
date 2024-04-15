#include "EngineLuaBindings.h"
#include "EntityInitLua.h"
#include "EntityItemHandlerLua.h"
#include "ItemInitLua.h"
#include "DialogueLua.h"
#include "DialogueChoiceConditionLua.h"
#include "World.h"
#include "Network.h"
#include "Interaction.h"
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
    DialogueLua& inDialogueLua,
    DialogueChoiceConditionLua& inDialogueChoiceConditionLua, World& inWorld,
    Network& inNetwork)
: entityInitLua{inEntityInitLua}
, entityItemHandlerLua{inEntityItemHandlerLua}
, itemInitLua{inItemInitLua}
, dialogueLua{inDialogueLua}
, dialogueChoiceConditionLua{inDialogueChoiceConditionLua}
, world{inWorld}
, network{inNetwork}
, dialogueChoiceLua{std::make_unique<sol::state>()}
, currentDialogueTopic{nullptr}
, workString{}
{
}

void EngineLuaBindings::addBindings()
{
    addEntityInitBindings();
    addEntityItemHandlerBindings();
    addItemInitBindings();
    addDialogueBindings();
    addDialogueChoiceConditionBindings();
    addDialogueChoiceBindings();
}

void EngineLuaBindings::addEntityInitBindings()
{
    entityInitLua.luaState.set_function(
        "addTalkInteraction", &EngineLuaBindings::addTalkInteraction, this);
    entityInitLua.luaState.set_function(
        "addItemHandler", &EngineLuaBindings::addItemHandler, this);
    entityInitLua.luaState.set_function("topic", &EngineLuaBindings::topic,
                                        this);
}

void EngineLuaBindings::addEntityItemHandlerBindings()
{
    entityItemHandlerLua.luaState.set_function(
        "addItemPlayer", [&](std::string_view itemID, Uint8 count) {
            return addItem(itemID, count, entityItemHandlerLua.clientEntity,
                           entityItemHandlerLua.clientID);
        });
    entityItemHandlerLua.luaState.set_function(
        "addItemSelf", [&](std::string_view itemID, Uint8 count) {
            return addItem(itemID, count, entityItemHandlerLua.targetEntity,
                           entityItemHandlerLua.clientID);
        });
    entityItemHandlerLua.luaState.set_function(
        "removeItemPlayer", [&](std::string_view itemID, Uint8 count) {
            return removeItem(itemID, count, entityItemHandlerLua.clientEntity,
                              entityItemHandlerLua.clientID);
        });
    entityItemHandlerLua.luaState.set_function(
        "removeItemSelf", [&](std::string_view itemID, Uint8 count) {
            return removeItem(itemID, count, entityItemHandlerLua.targetEntity,
                              entityItemHandlerLua.clientID);
        });
    entityItemHandlerLua.luaState.set_function(
        "getItemCountPlayer", [&](std::string_view itemID) {
            return getItemCount(itemID, entityItemHandlerLua.clientEntity,
                                entityItemHandlerLua.clientID);
        });
    entityItemHandlerLua.luaState.set_function(
        "getItemCountSelf", [&](std::string_view itemID) {
            return getItemCount(itemID, entityItemHandlerLua.targetEntity,
                                entityItemHandlerLua.clientID);
        });
    entityItemHandlerLua.luaState.set_function(
        "sendSystemMessage", [&](std::string_view message) {
            sendSystemMessage(message, entityItemHandlerLua.clientID);
        });
}

void EngineLuaBindings::addItemInitBindings()
{
    itemInitLua.luaState.set_function("setDescription",
                                      &EngineLuaBindings::setDescription, this);
    itemInitLua.luaState.set_function(
        "setMaxStackSize", &EngineLuaBindings::setMaxStackSize, this);
    itemInitLua.luaState.set_function("addCombination",
                                      &EngineLuaBindings::addCombination, this);
}

void EngineLuaBindings::addDialogueBindings()
{
    dialogueLua.luaState.set_function("say", &EngineLuaBindings::say, this);
    dialogueLua.luaState.set_function("narrate", &EngineLuaBindings::narrate,
                                      this);
    dialogueLua.luaState.set_function("wait", &EngineLuaBindings::wait, this);
    dialogueLua.luaState.set_function("setNextTopic",
                                      &EngineLuaBindings::setNextTopic, this);
    dialogueLua.luaState.set_function(
        "addItemPlayer", [&](std::string_view itemID, Uint8 count) {
            return addItem(itemID, count, dialogueLua.clientEntity,
                           dialogueLua.clientID);
        });
    dialogueLua.luaState.set_function(
        "addItemSelf", [&](std::string_view itemID, Uint8 count) {
            return addItem(itemID, count, dialogueLua.targetEntity,
                           dialogueLua.clientID);
        });
    dialogueLua.luaState.set_function(
        "removeItemPlayer", [&](std::string_view itemID, Uint8 count) {
            return removeItem(itemID, count, dialogueLua.clientEntity,
                              dialogueLua.clientID);
        });
    dialogueLua.luaState.set_function(
        "removeItemSelf", [&](std::string_view itemID, Uint8 count) {
            return removeItem(itemID, count, dialogueLua.targetEntity,
                              dialogueLua.clientID);
        });
    dialogueLua.luaState.set_function(
        "getItemCountPlayer", [&](std::string_view itemID) {
            return getItemCount(itemID, dialogueLua.clientEntity,
                                dialogueLua.clientID);
        });
    dialogueLua.luaState.set_function(
        "getItemCountSelf", [&](std::string_view itemID) {
            return getItemCount(itemID, dialogueLua.targetEntity,
                                dialogueLua.clientID);
        });
    dialogueLua.luaState.set_function(
        "sendSystemMessage", [&](std::string_view message) {
            sendSystemMessage(message, dialogueLua.clientID);
        });
}

void EngineLuaBindings::addDialogueChoiceConditionBindings()
{
    dialogueChoiceConditionLua.luaState.set_function(
        "getItemCountPlayer", [&](std::string_view itemID) {
            return getItemCount(itemID, dialogueChoiceConditionLua.clientEntity,
                                dialogueChoiceConditionLua.clientID);
        });
    dialogueChoiceConditionLua.luaState.set_function(
        "getItemCountSelf", [&](std::string_view itemID) {
            return getItemCount(itemID, dialogueChoiceConditionLua.targetEntity,
                                dialogueChoiceConditionLua.clientID);
        });
}

void EngineLuaBindings::addDialogueChoiceBindings()
{
    dialogueChoiceLua->set_function("choice", &EngineLuaBindings::choice, this);
    dialogueChoiceLua->set_function("choiceIf", &EngineLuaBindings::choiceIf,
                                    this);
}

void EngineLuaBindings::addTalkInteraction()
{
    Interaction& interaction{
        world.registry.get_or_emplace<Interaction>(entityInitLua.selfEntity)};
    if (!(interaction.add(EntityInteractionType::Talk))) {
        throw std::runtime_error{"Failed to add Talk interaction (already "
                                 "present or interaction limit reached)."};
    }
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
        workString.clear();
        workString.append("Failed to add topic \"");
        workString.append(topicName);
        workString.append("\". Topic limit reached (256)");
        throw std::runtime_error{workString};
    }

    // Add the new topic to the vector and the map.
    Dialogue::Topic& topic{dialogue.topics.emplace_back(
        std::string{topicName}, std::string{topicScript})};
    Uint8 topicIndex{static_cast<Uint8>(dialogue.topics.size() - 1)};
    dialogue.topicIndices.emplace(topicName, topicIndex);

    // Run the given choice script to fill the choices vector.
    currentDialogueTopic = &topic;
    auto result{
        dialogueChoiceLua->script(choiceScript, &sol::script_pass_on_error)};

    if (!(result.valid())) {
        sol::error err = result;
        workString.clear();
        workString.append("Error in choice script of topic \"");
        workString.append(topicName);
        workString.append("\": ");
        workString.append(err.what());
        throw std::runtime_error{workString};
    }
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
        workString.clear();
        workString.append("Failed to add item combination. Item(s) not found: ");
        if (!otherItem) {
            workString.append("\"");
            workString.append(otherItemID);
            workString.append("\" ");
        }
        if (!resultItem) {
            workString.append("\"");
            workString.append(resultItemID);
            workString.append("\" ");
        }

        throw std::runtime_error{workString};
    }
}

void EngineLuaBindings::say(std::string_view text)
{
    dialogueLua.dialogueEvents->emplace_back(SayEvent{std::string{text}});
}

void EngineLuaBindings::narrate(std::string_view text)
{
    dialogueLua.dialogueEvents->emplace_back(NarrateEvent{std::string{text}});
}

void EngineLuaBindings::wait(float timeS)
{
    dialogueLua.dialogueEvents->emplace_back(WaitEvent{timeS});
}

void EngineLuaBindings::setNextTopic(std::string_view topicName)
{
    // Note: The name's validity is checked after the script is finished.
    dialogueLua.nextTopicName = topicName;
}

void EngineLuaBindings::choice(std::string_view displayText,
                               std::string_view actionScript)
{
    currentDialogueTopic->choices.emplace_back("", std::string{displayText},
                                               std::string{actionScript});
}

void EngineLuaBindings::choiceIf(std::string_view conditionScript,
                                 std::string_view displayText,
                                 std::string_view actionScript)
{
    currentDialogueTopic->choices.emplace_back(std::string{conditionScript},
                                               std::string{displayText},
                                               std::string{actionScript});
}

bool EngineLuaBindings::addItem(std::string_view itemID, Uint8 count,
                                entt::entity entityToAddTo, NetworkID clientID)
{
    // Try to add the item, sending messages appropriately.
    // Note: We send errors to the user. It may confuse them, but at least
    //       they'll get some feedback.
    return InventoryHelpers::addItem(itemID, count, entityToAddTo, world,
                                     network, clientID);
}

bool EngineLuaBindings::removeItem(std::string_view itemID, Uint8 count,
                                   entt::entity entityToRemoveFrom,
                                   NetworkID clientID)
{
    // Try to remove the item, sending messages appropriately.
    // Note: This will walk the whole inventory, looking for enough copies of
    //       the item to satisfy the given count.
    // Note: We send errors to the user. It may confuse them, but at least
    //       they'll get some feedback.
    return InventoryHelpers::removeItem(itemID, count, entityToRemoveFrom,
                                        world, network, clientID);
}

std::size_t EngineLuaBindings::getItemCount(std::string_view itemID,
                                            entt::entity entityToCount,
                                            NetworkID clientID)
{
    // Try to return the count for the given item.
    const Item* item{world.itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToCount)};
    if (item && inventory) {
        return inventory->getItemCount(item->numericID);
    }
    else {
        network.serializeAndSend(
            clientID, SystemMessage{"Failed to get item count: invalid item ID "
                                    "or inventory doesn't exist."});
    }

    return 0;
}

void EngineLuaBindings::sendSystemMessage(std::string_view message,
                                          NetworkID clientID)
{
    network.serializeAndSend(clientID, SystemMessage{std::string{message}});
}

} // namespace Server
} // namespace AM
