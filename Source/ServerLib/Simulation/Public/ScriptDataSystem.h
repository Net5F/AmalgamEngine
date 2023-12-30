#pragma once

#include "EntityInitScriptRequest.h"
#include "ItemInitScriptRequest.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{
class World;
class Network;

/**
 * Handles sending Lua scripts to clients.
 *
 * Scripts are stored on the server in various components. When a client wants
 * to edit an entity, we send them that entity's scripts so they don't have to
 * start from scratch.
 */
class ScriptDataSystem
{
public:
    ScriptDataSystem(World& inWorld, Network& inNetwork);

    /**
     * Processes script data requests, sending script data if the request is
     * valid.
     */
    void sendScripts();

private:
    /** Used for fetching script data. */
    World& world;
    /** Used for receiving script requests and sending scripts to clients. */
    Network& network;

    /**
     * Sends the requested init script.
     */
    void sendEntityInitScript(const EntityInitScriptRequest& initScriptRequest);
    void sendItemInitScript(const ItemInitScriptRequest& initScriptRequest);

    EventQueue<EntityInitScriptRequest> entityInitScriptRequestQueue;
    EventQueue<ItemInitScriptRequest> itemInitScriptRequestQueue;
};

} // namespace Server
} // namespace AM
