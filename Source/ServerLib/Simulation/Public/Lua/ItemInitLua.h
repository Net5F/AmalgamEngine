#pragma once

#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
struct Item;

namespace Server
{

/**
 * The lua environment for running item init scripts.
 *
 * Contains additional members that are set by the script runner to pass 
 * relevant data to the environment's bound functions.
 */
struct ItemInitLua {
    /** Lua environment for item init script processing. */
    sol::state luaState{};

    /** The item that the init script is being ran on.
        Will always be non-nullptr while a script is running. */
    Item* selfItem{};
};

} // namespace Server
} // namespace AM
