#include "InteractionSystem.h"
#include "World.h"
#include "Network.h"
#include "Interaction.h"
#include "ItemDataRequest.h"
#include "AMAssert.h"
#include "Log.h"
#include <algorithm>

// TODO: Probably delete and revert InteractionManager
namespace AM
{
namespace Client
{
InteractionSystem::InteractionSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, inventoryInit{network.getEventDispatcher()}
, itemInteractionRequestQueue{network.getEventDispatcher()}
{
}

void InteractionSystem::processInteractions()
{
    // Send any waiting interaction requests from the UI to the server.
    sendWaitingInteractionRequests();

    // Process any received interaction updates.
    processInteractionUpdates();
}

void InteractionSystem::sendWaitingInteractionRequests()
{
    // Forward the requests to the server.
    EntityInteractionRequest entityInteractionRequest{};
    while (entityInteractionRequestQueue.pop(entityInteractionRequest)) {
        network.serializeAndSend(entityInteractionRequest);
    }

    ItemInteractionRequest itemInteractionRequest{};
    while (itemInteractionRequestQueue.pop(itemInteractionRequest)) {
        network.serializeAndSend(itemInteractionRequest);
    }
}

void InteractionSystem::processInteractionUpdates()
{
}

} // namespace Client
} // namespace AM
