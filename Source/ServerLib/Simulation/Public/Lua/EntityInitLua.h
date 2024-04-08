#pragma once

#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
namespace Server
{
/**
 * The lua environment for running entity init scripts.
 *
 * Contains additional members that are set by the script runner to pass 
 * relevant data to the environment's bound functions.
 */
struct EntityInitLua {
    /** Lua environment for entity init script processing. */
    sol::state luaState{};

    /** The entity that the init script is being ran on. */
    entt::entity selfEntity{};
};

} // namespace Server
} // namespace AM
