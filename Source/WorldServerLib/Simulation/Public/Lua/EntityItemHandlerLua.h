#pragma once

#include "ConnectionHandle.h"
#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
struct Item;

namespace WorldServer
{
/**
 * The lua environment for running entity item handler scripts.
 *
 * Contains additional members that are set by the script runner to pass
 * relevant data to the environment's bound functions.
 */
struct EntityItemHandlerLua {
    /** Lua environment for entity item handler script processing.
        Global variables:
          "self": The ID of the entity that the item is being used on.
          "target": The ID of the entity that used the item.
          "itemID": The string ID of the item that's being used.
          "GLOBAL": A constant used to identify the global value store.

        Note: Bound functions should use selfEntity and targetEntity instead
              of reading the globals, since scripts may overwrite them. */
    sol::state luaState{};

    /** The handle of the client that used the item. */
    ConnectionHandle clientHandle{};

    /** The entity that the item is being used on (the "self" global). */
    entt::entity selfEntity{};

    /** The entity that used the item (the "target" global). */
    entt::entity targetEntity{};

    /** The item that's being used.
        Will always be non-nullptr while a script is running. */
    const Item* item{};
};

} // namespace WorldServer
} // namespace AM
