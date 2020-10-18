#include "NetworkInputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "GameDefs.h"
#include "ClientInputs.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Server
{
NetworkInputSystem::NetworkInputSystem(Game& inGame, World& inWorld,
                                       Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
{
}

void NetworkInputSystem::processInputMessages()
{
    // Get the queue reference for this tick's messages.
    std::queue<std::unique_ptr<ClientInputs>>& messageQueue
        = network.startReceiveInputMessages(game.getCurrentTick());

    // Process all messages.
    while (!(messageQueue.empty())) {
        std::unique_ptr<ClientInputs> inputMessage
            = std::move(messageQueue.front());
        messageQueue.pop();
        if (inputMessage == nullptr) {
            LOG_INFO("Failed to receive input message after getting count "
                     "(this shouldn't happen).")
        }

        // Find the EntityID associated with the given NetID.
        Uint32 clientEntityID = 0;
        bool clientEntityFound = false;
        for (auto& element : world.clients) {
            if (element.second.netID == inputMessage->netID) {
                clientEntityID = element.first;
                clientEntityFound = true;
            }
        }

        if (!clientEntityFound) {
            LOG_ERROR(
                "Tried to process input message for netID that doesn't exist in the world.");
        }

        // Update the entity's InputComponent.
        world.inputs[clientEntityID] = inputMessage->inputComponent;

        // Flag the entity as dirty.
        world.entityIsDirty[clientEntityID] = true;
    }

    network.endReceiveInputMessages();
}

} // namespace Server
} // namespace AM
