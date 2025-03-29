#pragma once

#include "NetworkID.h"
#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
namespace Server
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
          "user": The ID of the entity that used the item.
                  May be a non-player entity.
          "self": The ID of the entity that the item is being used on.
          "GLOBAL": A constant used to identify the global value store. */
    sol::state luaState{};

    /** The network ID of the client that used the item. */
    NetworkID clientID{0};
};

} // namespace Server
} // namespace AM
