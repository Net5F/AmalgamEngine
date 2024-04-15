#pragma once

#include "ItemID.h"
#include "Dialogue.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <string_view>

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
class World;
class Network;

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
                      World& inWorld, Network& inNetwork);

    /**
     * Adds our bindings to the lua object.
     */
    void addBindings();

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
    // TODO: getFlagPlayer/Self, setFlagPlayer/Self

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
    // TODO: getFlagPlayer/Self, setFlagPlayer/Self

    /**
     * Adds a piece of dialogue to the dialogue event list.
     */
    void say(std::string_view text);

    /**
     * Adds a piece of narration to the dialogue event list.
     */
    void narrate(std::string_view text);

    /**
     * Adds a wait to the dialogue event list.
     * Note: The client will automatically add a wait between every say command.
     *       This command is to add an additional wait.
     */
    void wait(float timeS);

    /**
     * Sets the given topic as the next topic to navigate to.
     * Does not immediately jump--the rest of the current script will finish.
     * Calling this multiple times will overwrite previous calls.
     */
    void setNextTopic(std::string_view topicName);

    //-------------------------------------------------------------------------
    // Dialogue choice condition
    //-------------------------------------------------------------------------
    // TODO: getFlagPlayer/Self

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
    bool addItem(std::string_view itemID, Uint8 count,
                 entt::entity entityToAddTo, NetworkID clientID);

    /**
     * Attempts to remove the given item from the client entity's inventory.
     * @return true if the item was successfully removed, else false (inventory
     *         didn't contain the item).
     */
    bool removeItem(std::string_view itemID, Uint8 count,
                    entt::entity entityToRemoveFrom, NetworkID clientID);

    /**
     * Returns the count for the given item across all slots in the given 
     * entity's inventory.
     */
    std::size_t getItemCount(std::string_view itemID,
                             entt::entity entityToCount, NetworkID clientID);

    /**
     * Sends a system message to the client.
     */
    void sendSystemMessage(std::string_view message, NetworkID clientID);

    EntityInitLua& entityInitLua;
    EntityItemHandlerLua& entityItemHandlerLua;
    ItemInitLua& itemInitLua;
    DialogueLua& dialogueLua;
    DialogueChoiceConditionLua& dialogueChoiceConditionLua;
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
