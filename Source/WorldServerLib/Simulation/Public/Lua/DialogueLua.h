#pragma once

#include "DialogueEvent.h"
#include "ConnectionHandle.h"
#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
namespace WorldServer
{
/**
 * The lua environment for running both dialogue topic scripts and dialogue
 * choice action scripts.
 *
 * Contains additional members that are set by the script runner to pass
 * relevant data to the environment's bound functions.
 */
struct DialogueLua {
    /** Lua environment for dialogue topic and choice action script
        processing.
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

    /** (Out) The dialogue events that should be sent to the client. */
    std::vector<DialogueEvent>* dialogueEvents{};

    /** (Out) The topic specified by the latest setNextTopic().
        Will == "" if no setNextTopic() was called by the latest-ran script. */
    std::string nextTopicName{""};
};

} // namespace WorldServer
} // namespace AM
