#pragma once

#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
namespace Server
{
/**
 * The lua environment for running dialogue choice condition scripts.
 *
 * Condition scripts are much more limited than other dialogue scripts. They 
 * only have access to getters, and will always be made into the form:
 * "r = (given script)" where r must hold a boolean type after evaluation.
 *
 * Contains additional members that are set by the script runner to pass 
 * relevant data to the environment's bound functions.
 */
struct DialogueChoiceConditionLua {
    /** Lua environment for dialogue choice condition script processing.
        Global variables:
          "user": The ID of the entity that is controlling the dialogue.
          "self": The ID of the entity that is delivering the dialogue.
          "GLOBAL": A constant used to identify the global value store. */
    sol::state luaState{};

    /** The network ID of the client that is controlling the dialogue. */
    NetworkID clientID{0};
};

} // namespace Server
} // namespace AM
