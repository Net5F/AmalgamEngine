#pragma once

#include "ItemID.h"
#include "EngineCollisionLayerType.h"
#include "Dialogue.h"
#include "NetworkID.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <string_view>
#include <functional>

namespace sol
{
class state;
}

namespace AM
{
namespace Server
{
struct EntityInitLua;
struct EntityItemHandlerLua;
struct ItemInitLua;
struct DialogueLua;
struct DialogueChoiceConditionLua;
class GraphicData;
class World;
class Network;
class ItemData;

/**
 * Holds any functionality that the engine wants to expose to Lua.
 *
 * Note: This is a class instead of a set of free functions, because it's more
 *       convenient for the bound functions to have access to some state.
 */
class EngineLuaBindings
{
public:
    EngineLuaBindings(EntityInitLua& inEntityInitLua,
                      EntityItemHandlerLua& inEntityItemHandlerLua,
                      ItemInitLua& inItemInitLua, DialogueLua& inDialogueLua,
                      DialogueChoiceConditionLua& inDialogueChoiceConditionLua,
                      const GraphicData& inGraphicData,
                      const ItemData& inItemData, World& inWorld,
                      Network& inNetwork);

private:
    void addEntityInitBindings();
    void addEntityItemHandlerBindings();
    void addItemInitBindings();
    void addDialogueBindings();
    void addDialogueChoiceConditionBindings();
    void addDialogueChoiceBindings();

    //-------------------------------------------------------------------------
    // Entity init
    //-------------------------------------------------------------------------
    void setCollisionLayers(CollisionLayerBitSet collisionLayers);

    void setCollisionMask(CollisionLayerBitSet collisionMask);

    /**
     * Adds the "Talk" interaction to the entity.
     * Use the topic() Lua function to add dialogue.
     */
    void addTalkInteraction();

    /**
     * Sets the given handler to be called when the given item is used on the
     * entity.
     */
    void addItemHandler(std::string_view itemID,
                        std::string_view handlerScript);

    /**
     * Adds a new topic to the entity's dialogue tree.
     */
    void topic(std::string_view topicName, std::string_view topicScript,
               std::string_view choiceScript);

    //-------------------------------------------------------------------------
    // Entity item handler
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Item init
    //-------------------------------------------------------------------------
    /**
     * Adds the description text that's shown the item is examined.
     */
    void setDescription(std::string_view description);

    /**
     * Sets the max stack size, for when the item is stacked in an inventory.
     */
    void setMaxStackSize(Uint8 newMaxStackSize);

    /**
     * Adds a combination with the given item, which will result in a new item
     * (both inputs will be consumed).
     * @param otherItemID The item to combine with.
     * @param resultitemID The resulting item.
     */
    void addCombination(std::string_view otherItemID,
                        std::string_view resultItemID,
                        std::string_view description);

    //-------------------------------------------------------------------------
    // Dialogue
    //-------------------------------------------------------------------------
    /**
     * Adds a piece of dialogue to the dialogue event list.
     */
    void say(std::string_view text);

    /**
     * Adds a piece of narration to the dialogue event list.
     */
    void narrate(std::string_view text);

    /**
     * Sets the given topic as the next topic to navigate to.
     * Does not immediately jump--the rest of the current script will finish.
     * Calling this multiple times will overwrite previous calls.
     */
    void setNextTopic(std::string_view topicName);

    //-------------------------------------------------------------------------
    // Dialogue choice condition
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Dialogue choice
    //-------------------------------------------------------------------------
    /**
     * Adds a dialogue choice.
     * @param displayText The text to display for this choice.
     * @param actionScript The actions to run if this choice is successfully 
     *                     selected.
     */
    void choice(std::string_view displayText, std::string_view actionScript);

    /**
     * Adds a conditional dialogue choice.
     * The condition will be used both when checking if the choice should be 
     * sent, and when validating an incoming request to select the choice.
     * @param conditionScript The condition to check against.
     * @param displayText The text to display for this choice.
     * @param actionScript The actions to run if this choice is successfully 
     *                     selected.
     */
    void choiceIf(std::string_view conditionScript,
                  std::string_view displayText, std::string_view actionScript);

    //-------------------------------------------------------------------------
    // Shared
    //-------------------------------------------------------------------------
    /**
     * Attempts to add the given item to the first available slot in 
     * entityToAddTo's inventory.
     * @return true if the item was successfully added, else false (inventory
     *         didn't exist, inventory was full).
     */
    bool addItem(entt::entity entityToAddTo, std::string_view itemID,
                 Uint8 count);

    /**
     * Attempts to remove the given item from the client entity's inventory.
     * @return true if the item was successfully removed, else false (inventory
     *         didn't contain the item).
     */
    bool removeItem(entt::entity entityToRemoveFrom, std::string_view itemID,
                    Uint8 count);

    /**
     * Returns the count for the given item across all slots in the given 
     * entity's inventory.
     */
    std::size_t getItemCount(entt::entity entityToCount,
                             std::string_view itemID);

    /**
     * Adds a new value, or overwrites an existing value.
     *
     * If newValue == 0 (the default value), the value will be deleted.
     * 
     * Note: There's no type safety with stored values. If you call storeInt 
     *       on a value that was previously set as a bool, it will be  
     *       overwritten without issue.
     *
     * @param entity The entity to store the value to. If == entt::null, the 
     *               the value will be stored to the global store instead.
     * @param stringID The string ID of the value to add or overwrite.
     * @param newValue The new value to use.
     */
    void storeUint(entt::entity entity, std::string_view stringID,
                   Uint32 newValue);
    void storeBool(entt::entity entity, std::string_view stringID,
                   bool newValue);
    void storeInt(entt::entity entity, std::string_view stringID, int newValue);
    void storeFloat(entt::entity entity, std::string_view stringID,
                    float newValue);
    /** @param newValue A time in seconds, since 0 UTC (Jan 1, 1970). */
    void storeTime(entt::entity entity, std::string_view stringID,
                   Uint32 newValue);
    void storeBitSet(entt::entity entity, std::string_view stringID,
                     Uint32 newValue);
    /** @param bitToSet The bit to set, within the 32-bit stored value. Must be
                        within the range [0, 31]. */
    void storeBit(entt::entity entity, std::string_view stringID,
                  Uint8 bitToSet, bool newValue);

    /**
     * Gets a stored value.
     * 
     * Note: There's no type safety with stored values. If you call getStoredInt 
     *       on a value that was previously set as a bool, it will be returned 
     *       as an int without issue.
     *
     * @param entity The entity to get the value from. If == entt::null, the 
     *               the value will be retrieved from the global store instead.
     * @param stringID The string ID of the value to get.
     * @return The requested value. If not found, returns 0 (the default value 
     *         that the flag would have if it existed).
     */
    Uint32 getStoredUint(entt::entity entity, std::string_view stringID);
    bool getStoredBool(entt::entity entity, std::string_view stringID);
    int getStoredInt(entt::entity entity, std::string_view stringID);
    float getStoredFloat(entt::entity entity, std::string_view stringID);
    /** @return A time in seconds, since 0 UTC (Jan 1, 1970). */
    Uint32 getStoredTime(entt::entity entity, std::string_view stringID);
    Uint32 getStoredBitSet(entt::entity entity, std::string_view stringID);
    /** @param bitToGet The bit to get, within the 32-bit stored value. Must be
                        within the range [0, 31]. */
    bool getStoredBit(entt::entity entity, std::string_view stringID,
                      Uint8 bitToGet);

    /**
     * Sets a bit in a bit set to the given value.
     * Note: We take bitSet as a Uint32, since Lua doesn't have a matching 
     *       concept and it's more efficient to 
     *       
     */
    void setBit(std::reference_wrapper<Uint32> bitSet, Uint8 bitToSet,
                bool newValue);

    /**
     * Gets a bit from a bit set.
     */
    bool getBit(Uint32 bitSet, Uint8 bitToGet);

    /**
     * Returns the current time in seconds since 0 UTC (Jan 1, 1970).
     * Note: We define our own function instead of using Lua's os.time because 
     *       many of the os library functions are not safe to expose to users.
     */
    Uint32 getCurrentTime();

    /**
     * Sends a system message to the client.
     */
    void sendSystemMessage(std::string_view message, NetworkID clientID);

    EntityInitLua& entityInitLua;
    EntityItemHandlerLua& entityItemHandlerLua;
    ItemInitLua& itemInitLua;
    DialogueLua& dialogueLua;
    DialogueChoiceConditionLua& dialogueChoiceConditionLua;
    const GraphicData& graphicData;
    const ItemData& itemData;
    World& world;
    Network& network;

    /** Used to run dialogue choice scripts. Only supports the choice() function.
        Since its interface never needs to be extended, and this class is the 
        only one that uses it, we can keep it private to this class. */
    std::unique_ptr<sol::state> dialogueChoiceLua;

    /** If we're in the middle of running a dialogue choice script, this holds 
        the topic from the entity's Dialogue::topics that we're currently 
        adding to. */
    Dialogue::Topic* currentDialogueTopic;

    /** A scratch buffer used while processing strings. */
    std::string workString;
};

} // namespace Server
} // namespace AM
