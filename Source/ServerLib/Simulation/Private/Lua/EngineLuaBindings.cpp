#include "EngineLuaBindings.h"
#include "EntityInitLua.h"
#include "EntityItemHandlerLua.h"
#include "ItemInitLua.h"
#include "DialogueLua.h"
#include "DialogueChoiceConditionLua.h"
#include "GraphicData.h"
#include "ItemData.h"
#include "CollisionBitSets.h"
#include "World.h"
#include "Network.h"
#include "Interaction.h"
#include "ItemHandlers.h"
#include "Inventory.h"
#include "StoredValues.h"
#include "SystemMessage.h"
#include "ItemProperty.h"
#include "InventoryHelpers.h"
#include "sol/sol.hpp"
#include <time.h>

namespace AM
{
namespace Server
{

EngineLuaBindings::EngineLuaBindings(
    EntityInitLua& inEntityInitLua,
    EntityItemHandlerLua& inEntityItemHandlerLua, ItemInitLua& inItemInitLua,
    DialogueLua& inDialogueLua,
    DialogueChoiceConditionLua& inDialogueChoiceConditionLua,
    const GraphicData& inGraphicData, const ItemData& inItemData,
    World& inWorld, Network& inNetwork)
: entityInitLua{inEntityInitLua}
, entityItemHandlerLua{inEntityItemHandlerLua}
, itemInitLua{inItemInitLua}
, dialogueLua{inDialogueLua}
, dialogueChoiceConditionLua{inDialogueChoiceConditionLua}
, graphicData{inGraphicData}
, itemData{inItemData}
, world{inWorld}
, network{inNetwork}
, dialogueChoiceLua{std::make_unique<sol::state>()}
, currentDialogueTopic{nullptr}
, workString{}
{
    // Initialize the Lua environments.
    // Note: We only allow the base library, to avoid giving scripts unsafe 
    //       access to our system.
    entityInitLua.luaState.open_libraries(sol::lib::base);
    entityItemHandlerLua.luaState.open_libraries(sol::lib::base);
    itemInitLua.luaState.open_libraries(sol::lib::base);
    dialogueLua.luaState.open_libraries(sol::lib::base);
    dialogueChoiceConditionLua.luaState.open_libraries(sol::lib::base);

    // Add the GLOBAL Lua constant to the non-init environments.
    // Note: GLOBAL can be used instead of an entity ID when setting stored 
    //       values.
    Uint32 nullEntityID{entt::to_integral(entt::entity{entt::null})};
    entityItemHandlerLua.luaState["GLOBAL"] = nullEntityID;
    dialogueLua.luaState["GLOBAL"] = nullEntityID;
    dialogueChoiceConditionLua.luaState["GLOBAL"] = nullEntityID;

    // Add our bindings.
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
        "setCollisionLayers", &EngineLuaBindings::setCollisionLayers, this);
    entityInitLua.luaState.set_function(
        "setCollisionMask", &EngineLuaBindings::setCollisionMask, this);
    // Note: TerrainWall, ClientEntity, NonClientEntity are handled 
    //       automatically.
    //       Object is only useful in setCollisionMask
    entityInitLua.luaState["CLT_OBJECT"] = CollisionLayerType::Object;
    entityInitLua.luaState["CLT_BLOCK_COLLISION"]
        = CollisionLayerType::BlockCollision;
    entityInitLua.luaState["CLT_BLOCK_LOS"] = CollisionLayerType::BlockLoS;

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
        "addItem",
        [&](entt::entity entityToAddTo, std::string_view itemID, Uint8 count) {
            return addItem(entityToAddTo, itemID, count);
        });
    entityItemHandlerLua.luaState.set_function(
        "removeItem", [&](entt::entity entityToRemoveFrom,
                          std::string_view itemID, Uint8 count) {
            return removeItem(entityToRemoveFrom, itemID, count);
        });
    entityItemHandlerLua.luaState.set_function(
        "getItemCount",
        [&](entt::entity entityToCount, std::string_view itemID) {
            return getItemCount(entityToCount, itemID);
        });
    entityItemHandlerLua.luaState.set_function(
        "storeUint", &EngineLuaBindings::storeUint, this);
    entityItemHandlerLua.luaState.set_function(
        "storeBool", &EngineLuaBindings::storeBool, this);
    entityItemHandlerLua.luaState.set_function(
        "storeInt", &EngineLuaBindings::storeInt, this);
    entityItemHandlerLua.luaState.set_function(
        "storeFloat", &EngineLuaBindings::storeFloat, this);
    entityItemHandlerLua.luaState.set_function(
        "storeTime", &EngineLuaBindings::storeTime, this);
    entityItemHandlerLua.luaState.set_function(
        "storeBitSet", &EngineLuaBindings::storeBitSet, this);
    entityItemHandlerLua.luaState.set_function(
        "storeBit", &EngineLuaBindings::storeBit, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredUint", &EngineLuaBindings::getStoredUint, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredBool", &EngineLuaBindings::getStoredBool, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredInt", &EngineLuaBindings::getStoredInt, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredFloat", &EngineLuaBindings::getStoredFloat, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredTime", &EngineLuaBindings::getStoredTime, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredBitSet", &EngineLuaBindings::getStoredBitSet, this);
    entityItemHandlerLua.luaState.set_function(
        "getStoredBit", &EngineLuaBindings::getStoredBit, this);
    entityItemHandlerLua.luaState.set_function(
        "getBit", &EngineLuaBindings::getBit, this);
    entityItemHandlerLua.luaState.set_function(
        "setBit", &EngineLuaBindings::setBit, this);
    entityItemHandlerLua.luaState.set_function(
        "getCurrentTime", &EngineLuaBindings::getCurrentTime, this);
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
        "addItem",
        [&](entt::entity entityToAddTo, std::string_view itemID, Uint8 count) {
            return addItem(entityToAddTo, itemID, count);
        });
    dialogueLua.luaState.set_function(
        "removeItem", [&](entt::entity entityToRemoveFrom,
                          std::string_view itemID, Uint8 count) {
            return removeItem(entityToRemoveFrom, itemID, count);
        });
    dialogueLua.luaState.set_function(
        "getItemCount",
        [&](entt::entity entityToCount, std::string_view itemID) {
            return getItemCount(entityToCount, itemID);
        });
    dialogueLua.luaState.set_function("storeUint",
                                      &EngineLuaBindings::storeUint, this);
    dialogueLua.luaState.set_function("storeBool",
                                      &EngineLuaBindings::storeBool, this);
    dialogueLua.luaState.set_function("storeInt", &EngineLuaBindings::storeInt,
                                      this);
    dialogueLua.luaState.set_function("storeFloat",
                                      &EngineLuaBindings::storeFloat, this);
    dialogueLua.luaState.set_function("storeTime",
                                      &EngineLuaBindings::storeTime, this);
    dialogueLua.luaState.set_function("storeBitSet",
                                      &EngineLuaBindings::storeBitSet, this);
    dialogueLua.luaState.set_function("storeBit", &EngineLuaBindings::storeBit,
                                      this);
    dialogueLua.luaState.set_function("getStoredUint",
                                      &EngineLuaBindings::getStoredUint, this);
    dialogueLua.luaState.set_function("getStoredBool",
                                      &EngineLuaBindings::getStoredBool, this);
    dialogueLua.luaState.set_function("getStoredInt",
                                      &EngineLuaBindings::getStoredInt, this);
    dialogueLua.luaState.set_function("getStoredFloat",
                                      &EngineLuaBindings::getStoredFloat, this);
    dialogueLua.luaState.set_function("getStoredTime",
                                      &EngineLuaBindings::getStoredTime, this);
    dialogueLua.luaState.set_function(
        "getStoredBitSet", &EngineLuaBindings::getStoredBitSet, this);
    dialogueLua.luaState.set_function("getStoredBit",
                                      &EngineLuaBindings::getStoredBit, this);
    dialogueLua.luaState.set_function("getBit", &EngineLuaBindings::getBit,
                                      this);
    dialogueLua.luaState.set_function("setBit", &EngineLuaBindings::setBit,
                                      this);
    dialogueLua.luaState.set_function("getCurrentTime",
                                      &EngineLuaBindings::getCurrentTime, this);
    dialogueLua.luaState.set_function(
        "sendSystemMessage", [&](std::string_view message) {
            sendSystemMessage(message, dialogueLua.clientID);
        });
}

void EngineLuaBindings::addDialogueChoiceConditionBindings()
{
    dialogueChoiceConditionLua.luaState.set_function(
        "getItemCount",
        [&](entt::entity entityToCount, std::string_view itemID) {
            return getItemCount(entityToCount, itemID);
        });
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredUint", &EngineLuaBindings::getStoredUint, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredBool", &EngineLuaBindings::getStoredBool, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredInt", &EngineLuaBindings::getStoredInt, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredFloat", &EngineLuaBindings::getStoredFloat, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredTime", &EngineLuaBindings::getStoredTime, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredBitSet", &EngineLuaBindings::getStoredBitSet, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getStoredBit", &EngineLuaBindings::getStoredBit, this);
    dialogueChoiceConditionLua.luaState.set_function(
        "getCurrentTime", &EngineLuaBindings::getCurrentTime, this);
}

void EngineLuaBindings::addDialogueChoiceBindings()
{
    dialogueChoiceLua->set_function("choice", &EngineLuaBindings::choice, this);
    dialogueChoiceLua->set_function("choiceIf", &EngineLuaBindings::choiceIf,
                                    this);
}

void EngineLuaBindings::setCollisionLayers(CollisionLayerBitSet collisionLayers)
{
    entt::entity selfEntity{entityInitLua.selfEntity};
    if (world.registry.all_of<CollisionBitSets>(selfEntity))
    {
        world.registry.patch<CollisionBitSets>(
            selfEntity, [&](CollisionBitSets& collisionBitSets) {
                collisionBitSets.setCollisionLayers(collisionLayers, selfEntity,
                                                    world.registry);
            });
    }
    else {
        throw std::runtime_error{"Failed to set collision mask: Entity does "
                                 "not have graphics components."};
    }
}

void EngineLuaBindings::setCollisionMask(CollisionLayerBitSet collisionMask)
{
    entt::entity selfEntity{entityInitLua.selfEntity};
    if (world.registry.all_of<CollisionBitSets>(selfEntity))
    {
        world.registry.patch<CollisionBitSets>(
            selfEntity, [&](CollisionBitSets& collisionBitSets) {
                collisionBitSets.setCollisionMask(collisionMask);
            });
    }
    else {
        throw std::runtime_error{"Failed to set collision mask: Entity does "
                                 "not have graphics components."};
    }
}

void EngineLuaBindings::addTalkInteraction()
{
    Interaction& interaction{
        world.registry.get_or_emplace<Interaction>(entityInitLua.selfEntity)};
    if (!(interaction.add(EntityInteractionType::Talk))) {
        throw std::runtime_error{"Failed to add Talk interaction: Already "
                                 "present or interaction limit reached."};
    }
}

void EngineLuaBindings::addItemHandler(std::string_view itemID,
                                       std::string_view handlerScript)
{
    // If the given item exists, add the given handler.
    if (const Item * item{itemData.getItem(itemID)}) {
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
        workString.append("\": Topic limit reached (256)");
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

    // Update the description.
    item->description = description;
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
    const Item* otherItem{itemData.getItem(otherItemID)};
    const Item* resultItem{itemData.getItem(resultItemID)};
    if (otherItem && resultItem) {
        Item* item{itemInitLua.selfItem};
        item->itemCombinations.emplace_back(otherItem->numericID,
                                            resultItem->numericID,
                                            std::string{description});
    }
    else {
        workString.clear();
        workString.append("Failed to add item combination: Item(s) not found: ");
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
    // Note: The name's validity is checked by DialogueSystem::runChoice, after
    //       the script is ran.
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

bool EngineLuaBindings::addItem(entt::entity entityToAddTo,
                                std::string_view itemID, Uint8 count)
{
    if (!(world.registry.valid(entityToAddTo))) {
        throw std::runtime_error{"Failed to add item: Invalid entity ID."};
    }

    // Try to add the item, sending update messages appropriately.
    auto result{InventoryHelpers::addItem(itemID, count, entityToAddTo,
                                          itemData, world, network)};
    if (result == InventoryHelpers::AddResult::InventoryFull) {
        throw std::runtime_error{"Failed to add item: Inventory is full."};
    }
    else if (result == InventoryHelpers::AddResult::ItemNotFound) {
        workString.clear();
        workString.append("Failed to add item: Item \"");
        workString.append(itemID);
        workString.append("\" not found.");
        throw std::runtime_error{workString};
    }

    return (result == InventoryHelpers::AddResult::Success);
}

bool EngineLuaBindings::removeItem(entt::entity entityToRemoveFrom,
                                   std::string_view itemID, Uint8 count)
{
    if (!(world.registry.valid(entityToRemoveFrom))) {
        throw std::runtime_error{"Failed to remove item: Invalid entity ID."};
    }

    // Try to remove the item, sending update messages appropriately.
    // Note: This will walk the whole inventory, looking for enough copies of
    //       the item to satisfy the given count.
    auto result{InventoryHelpers::removeItem(itemID, count, entityToRemoveFrom,
                                             world, network, itemData)};
    if (result == InventoryHelpers::RemoveResult::InsufficientItemCount) {
        throw std::runtime_error{
            "Failed to remove item: Insufficient item count."};
    }
    else if (result == InventoryHelpers::RemoveResult::InventoryNotFound) {
        throw std::runtime_error{"Failed to remove item: Entity has no "
                                 "Inventory component."};
    }
    else if (result == InventoryHelpers::RemoveResult::ItemNotFound) {
        workString.clear();
        workString.append("Failed to remove item: Item \"");
        workString.append(itemID);
        workString.append("\" not found.");
        throw std::runtime_error{workString};
    }

    return (result == InventoryHelpers::RemoveResult::Success);
}

std::size_t EngineLuaBindings::getItemCount(entt::entity entityToCount,
                                            std::string_view itemID)
{
    if (!(world.registry.valid(entityToCount))) {
        throw std::runtime_error{
            "Failed to get item count: Invalid entity ID."};
    }

    // Try to return the count for the given item.
    const Item* item{itemData.getItem(itemID)};
    auto* inventory{world.registry.try_get<Inventory>(entityToCount)};
    if (item && inventory) {
        return inventory->getItemCount(item->numericID);
    }
    else if (inventory) {
        throw std::runtime_error{"Failed to get item count: Invalid item ID."};
    }

    // Inventory doesn't exist, so the entity has 0 count of the item by default.
    return 0;
}

void EngineLuaBindings::storeUint(entt::entity entity,
                                  std::string_view stringID, Uint32 newValue)
{
    // If we were given a non-null entity, use its store.
    if (entity != entt::null) {
        if (!(world.registry.valid(entity))) {
            throw std::runtime_error{
                "Failed to store value: Invalid entity ID."};
        }

        // Try to store the value.
        StoredValues& storedValues{
            world.registry.get_or_emplace<StoredValues>(entity)};
        if (!(storedValues.storeValue(stringID, newValue, world))) {
            workString.clear();
            workString.append("Failed to store value \"");
            workString.append(stringID);
            workString.append("\": value doesn't exist and value limit is reached");
            throw std::runtime_error{workString};
        }
    }
    else {
        // We were given entt::null, use the global store.
        world.storeGlobalValue(stringID, newValue);
    }
}

void EngineLuaBindings::storeBool(entt::entity entity,
                                  std::string_view stringID, bool newValue)
{
    storeUint(entity, stringID, static_cast<Uint32>(newValue));
}

void EngineLuaBindings::storeInt(entt::entity entity, std::string_view stringID,
                                 int newValue)
{
    storeUint(entity, stringID, static_cast<Uint32>(newValue));
}

void EngineLuaBindings::storeFloat(entt::entity entity,
                                   std::string_view stringID, float newValue)
{
    static_assert(sizeof(float) == 4, "float is expected to be 4 bytes.");

    // Copy the float's bytes to a Uint32 without converting.
    Uint32 newValueUint{};
    std::memcpy(&newValueUint, &newValue, 4);
    storeUint(entity, stringID, newValueUint);
}

void EngineLuaBindings::storeTime(entt::entity entity,
                                  std::string_view stringID, Uint32 newValue)
{
    // Note: Time is only a type to make scripts more readable. It's handled 
    //       the same as Uint32.
    storeUint(entity, stringID, newValue);
}

void EngineLuaBindings::storeBitSet(entt::entity entity,
                                    std::string_view stringID, Uint32 newValue)
{
    // Note: BitSet is only a type to make scripts more readable. It's handled 
    //       the same as Uint32.
    storeUint(entity, stringID, newValue);
}

void EngineLuaBindings::storeBit(entt::entity entity, std::string_view stringID,
                                 Uint8 bitToSet, bool newValue)
{
    if (bitToSet > 31) {
        throw std::runtime_error{
            "Failed to store bit: bitToSet must be within range [0, 31]."};
    }

    // This will give us either the current value if it already exists, or 
    // 0 (a fresh bit set).
    Uint32 bitSet{getStoredUint(entity, stringID)};

    // Set the bit and store the new value.
    setBit(bitSet, bitToSet, newValue);

    storeUint(entity, stringID, bitSet);
}

Uint32 EngineLuaBindings::getStoredUint(entt::entity entity,
                                        std::string_view stringID)
{
    // If we were given a non-null entity, use its store.
    if (entity != entt::null) {
        if (!(world.registry.valid(entity))) {
            throw std::runtime_error{
                "Failed to get stored value: Invalid entity ID."};
        }

        // Try to get the value.
        if (auto* storedValues{world.registry.try_get<StoredValues>(entity)}) {
            return storedValues->getStoredValue(stringID, world);
        }
    }
    else {
        // We were given entt::null, use the global store.
        return world.getStoredValue(stringID);
    }

    return 0;
}

bool EngineLuaBindings::getStoredBool(entt::entity entity,
                                      std::string_view stringID)
{
    return static_cast<bool>(getStoredUint(entity, stringID));
}

int EngineLuaBindings::getStoredInt(entt::entity entity,
                                    std::string_view stringID)
{
    return static_cast<int>(getStoredUint(entity, stringID));
}

float EngineLuaBindings::getStoredFloat(entt::entity entity,
                                        std::string_view stringID)
{
    // Copy the stored Uint32's bytes to a float without converting.
    Uint32 storedValue{getStoredUint(entity, stringID)};
    float storedValueFloat{};
    std::memcpy(&storedValueFloat, &storedValue, 4);

    return storedValueFloat;
}

Uint32 EngineLuaBindings::getStoredTime(entt::entity entity,
                                        std::string_view stringID)
{
    // Note: Time is only a type to make scripts more readable. It's handled 
    //       the same as Uint32.
    return getStoredUint(entity, stringID);
}

Uint32 EngineLuaBindings::getStoredBitSet(entt::entity entity,
                                                   std::string_view stringID)
{
    // Note: BitSet is only a type to make scripts more readable. It's handled 
    //       the same as Uint32.
    return getStoredUint(entity, stringID);
}

bool EngineLuaBindings::getStoredBit(entt::entity entity,
                                     std::string_view stringID, Uint8 bitToGet)
{
    if (bitToGet > 31) {
        throw std::runtime_error{
            "Failed to get bit: bitToGet must be within range [0, 31]."};
    }

    // This will give us either the current value if it already exists, or 
    // 0 (a fresh bit set).
    Uint32 storedValue{getStoredUint(entity, stringID)};

    return getBit(storedValue, bitToGet);
}

void EngineLuaBindings::setBit(std::reference_wrapper<Uint32> bitSet,
                               Uint8 bitToSet, bool newValue)
{
    if (bitToSet > 31) {
        throw std::runtime_error{
            "Failed to set bit: bitToSet must be within range [0, 31]."};
    }

    Uint32 mask{static_cast<Uint32>(1) << bitToSet};
    if (newValue) {
        bitSet |= mask;
    }
    else {
        bitSet &= ~mask;
    }
}

bool EngineLuaBindings::getBit(Uint32 bitSet, Uint8 bitToGet)
{
    if (bitToGet > 31) {
        throw std::runtime_error{
            "Failed to get bit: bitToGet must be within range [0, 31]."};
    }

    return (bitSet >> bitToGet) & static_cast<Uint32>(1);
}

Uint32 EngineLuaBindings::getCurrentTime()
{
    time_t timeValue{time(nullptr)};
    return static_cast<Uint32>(timeValue);
}

void EngineLuaBindings::sendSystemMessage(std::string_view message,
                                          NetworkID clientID)
{
    network.serializeAndSend(clientID, SystemMessage{std::string{message}});
}

} // namespace Server
} // namespace AM
