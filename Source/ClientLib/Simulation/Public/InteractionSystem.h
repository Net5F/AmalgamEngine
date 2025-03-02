#pragma once

#include "EntityInteractionRequest.h"
#include "ItemInteractionRequest.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
class World;
class Network;

/**
 * Processes entity and item interactions (e.g., the verbs that you see when 
 * you right-click an item or entity).
 *
 * Interactions originate in the UI, but all other processing happens here.
 */
class InteractionSystem
{
public:
    InteractionSystem(World& inWorld, Network& inNetwork);

    /**
     * Forwards waiting interaction requests from the UI to the server, and 
     * processes waiting interaction updates.
     */
    void processInteractions();

private:
    /**
     * Forwards waiting UI requests to the server.
     */
    void sendWaitingInteractionRequests();

    /**
     * Processes any received interaction update messages.
     */
    void processInteractionUpdates();

    /** Used for processing interactions. */
    World& world;
    /** Used for sending requests and receiving interaction data. */
    Network& network;

    // UI requests
    EventQueue<EntityInteractionRequest> entityInteractionRequestQueue;
    EventQueue<ItemInteractionRequest> itemInteractionRequestQueue;

    // Server responses
};

} // namespace Client
} // namespace AM
