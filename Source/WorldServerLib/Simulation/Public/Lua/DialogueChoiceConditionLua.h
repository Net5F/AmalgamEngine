#pragma once

#include "ConnectionHandle.h"
#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
namespace WorldServer
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
          "self": The ID of the entity that is delivering the dialogue.
          "target": The ID of the entity that is controlling the dialogue.
          "GLOBAL": A constant used to identify the global value store.

        Note: Bound functions should use selfEntity and targetEntity instead
              of reading the globals, since scripts may overwrite them. */
    sol::state luaState{};

    /** The entity that is delivering the dialogue (the "self" global). */
    entt::entity selfEntity{};

    /** The entity that is controlling the dialogue (the "target" global). */
    entt::entity targetEntity{};

    /** The handle of the client that is controlling the dialogue. */
    ConnectionHandle clientHandle{};
};

} // namespace WorldServer
} // namespace AM
