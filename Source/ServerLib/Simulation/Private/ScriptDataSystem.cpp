#include "ScriptDataSystem.h"
#include "World.h"
#include "Network.h"
#include "EntityInitScriptResponse.h"
#include "ItemInitScriptResponse.h"
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
, entityInitScriptRequestQueue{inNetwork.getEventDispatcher()}
, itemInitScriptRequestQueue{inNetwork.getEventDispatcher()}
{
}

void ScriptDataSystem::sendScripts()
{
    // Process all script data requests.
    EntityInitScriptRequest entityInitScriptRequest{};
    while (entityInitScriptRequestQueue.pop(entityInitScriptRequest)) {
        sendEntityInitScript(entityInitScriptRequest);
    }

    ItemInitScriptRequest itemInitScriptRequest{};
    while (itemInitScriptRequestQueue.pop(itemInitScriptRequest)) {
        sendItemInitScript(itemInitScriptRequest);
    }
}

void ScriptDataSystem::sendEntityInitScript(
    const EntityInitScriptRequest& initScriptRequest)
{
    // If the given entity has an init script, send it.
    entt::entity entity{initScriptRequest.entity};
    if (const auto* initScript{
            world.registry.try_get<EntityInitScript>(entity)}) {
        network.serializeAndSend(initScriptRequest.netID,
                                 EntityInitScriptResponse{entity, *initScript});
    }
}

void ScriptDataSystem::sendItemInitScript(
    const ItemInitScriptRequest& initScriptRequest)
{
    // If the given item is valid, send its init script.
    if (const Item* item{world.itemData.getItem(initScriptRequest.itemID)}) {
        network.serializeAndSend(
            initScriptRequest.netID,
            ItemInitScriptResponse{item->numericID, {item->initScript}});
    }
}

} // End namespace Server
} // End namespace AM
