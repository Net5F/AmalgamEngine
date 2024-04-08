#pragma once

#include "NetworkDefs.h"
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
    /** Lua environment for entity item handler script processing. */
    sol::state luaState{};

    /** The network ID of the client that used the item. */
    NetworkID clientID{0};

    /** The entity that used the item. */
    entt::entity clientEntity{};

    /** The entity that the item is being used on. */
    entt::entity targetEntity{};
};

} // namespace Server
} // namespace AM
