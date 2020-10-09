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

        // Update the entity's InputComponent.
        Uint32 clientEntityID = inputMessage->id;
        world.inputs[clientEntityID] = inputMessage->inputComponent;

        // Flag the entity as dirty.
        world.entityIsDirty[clientEntityID] = true;

        LOG_INFO("Processed input message on tick %u. Message tick: %u",
                  game.getCurrentTick(), inputMessage->tickNum);
    }

    network.endReceiveInputMessages();
}

} // namespace Server
} // namespace AM
