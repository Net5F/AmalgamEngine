#pragma once

#include "ItemID.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <string>
#include <string_view>

namespace sol
{
class state;
}

namespace AM
{
namespace Server
{
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
    EngineLuaBindings(sol::state& inEntityInitLua,
                      sol::state& inEntityItemHandlerLua,
                      sol::state& inItemInitLua, World& inWorld,
                      Network& inNetwork);

    /**
     * Adds our bindings to the lua object.
     */
    void addBindings();

private:
    sol::state& entityInitLua;
    sol::state& entityItemHandlerLua;
    sol::state& itemInitLua;
    World& world;
    Network& network;

    // Entity init
    /**
     * Sets the given handler to be called when the given item is used on the
     * entity.
     */
    void addItemHandler(const std::string& itemID,
                        const std::string& handlerScript);

    // Entity item handler
    /**
     * Attempts to add the given item to the first available slot in the client
     * entity's inventory.
     * @return true if the item was successfully added, else false (inventory
     *         was full).
     */
    bool addItem(const std::string& itemID, Uint8 count);

    /**
     * Attempts to remove the given item from the client entity's inventory.
     * @return true if the item was successfully removed, else false (inventory
     *         didn't contain the item).
     */
    bool removeItem(const std::string& itemID, Uint8 count);

    /**
     * Returns the count for the given item across all slots in the client 
     * entity's inventory.
     */
    std::size_t getItemCount(ItemID itemID);

    /**
     * Sends a system message to the client.
     */
    void sendSystemMessage(const std::string& message);

    // Item init
    /**
     * Adds the description text that's shown the item is examined.
     */
    void setDescription(const std::string& description);

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
    void addCombination(const std::string& otherItemID,
                        const std::string& resultItemID,
                        const std::string& description);
};

} // namespace Server
} // namespace AM
