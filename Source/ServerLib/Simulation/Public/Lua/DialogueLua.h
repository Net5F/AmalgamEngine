#pragma once

#include "DialogueEvent.h"
#include "sol/sol.hpp"
#include "entt/fwd.hpp"

namespace AM
{
namespace Server
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
          "user": The ID of the entity that is controlling the dialogue.
          "self": The ID of the entity that is delivering the dialogue.
          "GLOBAL": A constant used to identify the global value store. */
    sol::state luaState{};

    /** The network ID of the client that is controlling the dialogue. */
    NetworkID clientID{0};

    /** (Out) The dialogue events that should be sent to the client. */
    std::vector<DialogueEvent>* dialogueEvents{};

    /** (Out) The topic specified by the latest setNextTopic().
        Will == "" if no setNextTopic() was called by the latest-ran script. */
    std::string nextTopicName{""};
};

} // namespace Server
} // namespace AM
