#pragma once

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

/**
 * Holds any functionality that the engine wants to expose to Lua.
 * 
 * Note: This is a class instead of a set of free functions, because it's more 
 *       convenient for the bound functions to have access to some state.
 */
class EngineLuaBindings
{
public:
    EngineLuaBindings(sol::state& inEntityInitLua, sol::state& inItemInitLua,
                      World& inWorld);

    /**
     * Adds our bindings to the lua object.
     */
    void addBindings();

private:
    sol::state& entityInitLua;
    sol::state& itemInitLua;
    World& world;

    // Entity init

    // Item init
    /**
     * Adds the description text for when an item is examined.
     */
    void addDescription(const std::string& description);

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
