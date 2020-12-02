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
    // Handle any dropped messages.
    processMessageDropEvents();

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
        Uint32 clientEntityID = world.findEntityWithNetID(inputMessage->netID);

        // Update the client entity's inputs.
        if (clientEntityID != INVALID_ENTITY_ID) {
            // Update the entity's InputComponent.
            world.inputs[clientEntityID] = inputMessage->inputComponent;

            // Flag the entity as dirty.
            world.entityIsDirty[clientEntityID] = true;
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
            EntityID clientEntityID
                = world.findEntityWithNetID(clientNetworkID);
            if (clientEntityID != INVALID_ENTITY_ID) {
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

void NetworkInputSystem::handleDropForEntity(EntityID entityID)
{
    // If the entity's inputs aren't default, mark it as dirty.
    InputComponent defaultInputs{};
    if (world.inputs[entityID].inputStates != defaultInputs.inputStates) {
        world.entityIsDirty[entityID] = true;
    }

    // Default the entity's inputs.
    world.inputs[entityID].inputStates = defaultInputs.inputStates;

    // Flag that a drop occurred for this entity.
    world.clients[entityID].messageWasDropped = true;
}

} // namespace Server
} // namespace AM
