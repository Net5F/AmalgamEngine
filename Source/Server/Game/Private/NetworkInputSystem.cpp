#include "NetworkInputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "GameDefs.h"
#include "ClientInput.h"
#include "Input.h"
#include "ClientState.h"
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
    // Handle any dropped messages.
    processMessageDropEvents();

    // Get the queue reference for this tick's messages.
    std::queue<std::unique_ptr<ClientInput>>& messageQueue
        = network.startReceiveInputMessages(game.getCurrentTick());

    // Process all messages.
    while (!(messageQueue.empty())) {
        std::unique_ptr<ClientInput> inputMessage
            = std::move(messageQueue.front());
        messageQueue.pop();
        if (inputMessage == nullptr) {
            LOG_INFO("Failed to receive input message after getting count "
                     "(this shouldn't happen).")
        }

        // Find the entity associated with the given NetID.
        entt::entity clientEntity = world.findEntityWithNetID(inputMessage->netID);

        // Update the client entity's inputs.
        if (clientEntity != entt::null) {
            // Update the entity's Input component.
            Input& input = world.registry.get<Input>(clientEntity);
            input = inputMessage->input;

            // Flag the entity as dirty.
            input.isDirty = true;
        }
        else {
            // The entity was probably disconnected. Do nothing with the
            // message.
        }
    }

    network.endReceiveInputMessages();
}

void NetworkInputSystem::processMessageDropEvents()
{
    // Get the queue reference for any dropped message events.
    moodycamel::ReaderWriterQueue<NetworkID>& messageDropEventQueue
        = network.getMessageDropEventQueue();

    // Process any message drop events.
    for (unsigned int i = 0; i < messageDropEventQueue.size_approx(); ++i) {
        // Pop the NetworkID of the client that we dropped a message from.
        NetworkID clientNetworkID = 0;
        if (messageDropEventQueue.try_dequeue(clientNetworkID)) {
            // Find the EntityID associated with the popped NetworkID.
            entt::entity clientEntityID
                = world.findEntityWithNetID(clientNetworkID);
            if (clientEntityID != entt::null) {
                // We found the entity that dropped the message, handle it.
                handleDropForEntity(clientEntityID);
            }
            else {
                LOG_ERROR("Failed to find entity with netID: %u while "
                          "processing a message drop event.",
                          clientNetworkID);
            }
        }
        else {
            LOG_ERROR("Expected element in messageDropEventQueue but dequeue "
                      "failed.");
        }
    }
}

void NetworkInputSystem::handleDropForEntity(entt::entity entity)
{
    // If the entity's inputs aren't default, mark it as dirty.
    entt::registry& registry = world.registry;
    Input& entityInput = registry.get<Input>(entity);
    Input defaultInput{};
    if (entityInput.inputStates != defaultInput.inputStates) {
        entityInput.isDirty = true;
    }

    // Default the entity's inputs.
    entityInput.inputStates = defaultInput.inputStates;

    // Flag that a drop occurred for this entity.
    registry.get<ClientState>(entity).messageWasDropped = true;
}

} // namespace Server
} // namespace AM
