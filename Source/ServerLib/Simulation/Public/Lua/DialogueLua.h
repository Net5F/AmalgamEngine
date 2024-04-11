#pragma once

#include "DialogueResponse.h"
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
        processing. */
    sol::state luaState{};

    /** The client entity that is controlling the dialogue. */
    entt::entity clientEntity{};

    /** The entity that is being talked to. */
    entt::entity targetEntity{};

    /** (Out) The dialogue events that should be sent to the client. */
    std::vector<DialogueResponse::DialogueEvent>* dialogueEvents{};

    /** (Out) The topic specified by the latest goto().
        Will == "" if no goto() was called by the latest-ran script. */
    std::string gotoTopicName{""};
};

} // namespace Server
} // namespace AM
