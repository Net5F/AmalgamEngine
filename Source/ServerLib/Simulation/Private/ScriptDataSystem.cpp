#include "ScriptDataSystem.h"
#include "World.h"
#include "Network.h"
#include "InitScript.h"
#include "InitScriptResponse.h"
#include "Log.h"
#include <SDL_rect.h>
#include <vector>

namespace AM
{
namespace Server
{
ScriptDataSystem::ScriptDataSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, initScriptRequestQueue{inNetwork.getEventDispatcher()}
{
}

void ScriptDataSystem::sendScripts()
{
    // Process all script data requests.
    InitScriptRequest initScriptRequest{};
    while (initScriptRequestQueue.pop(initScriptRequest)) {
        sendInitScript(initScriptRequest);
    }
}

void ScriptDataSystem::sendInitScript(
    const InitScriptRequest& initScriptRequest)
{
    // If the given entity has an init script, send it.
    entt::entity entity{initScriptRequest.entity};
    if (world.registry.all_of<InitScript>(entity)) {
        const auto& initScript{world.registry.get<InitScript>(entity)};
        network.serializeAndSend(initScriptRequest.netID,
                                 InitScriptResponse{entity, initScript.script});
    }
}

} // End namespace Server
} // End namespace AM
