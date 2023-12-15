#pragma once

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
};

} // namespace Server
} // namespace AM
