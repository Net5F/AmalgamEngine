#include "NetworkInputSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "ClientInput.h"
#include "Input.h"
#include "IsDirty.h"
#include "ClientSimData.h"
#include "Log.h"
#include <memory>
#include "Profiler.h"

namespace AM
{
namespace Server
{
NetworkInputSystem::NetworkInputSystem(Simulation& inSim, World& inWorld,
                                       Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
, messageDroppedQueue(inNetwork.getDispatcher())
{
}

void NetworkInputSystem::processInputMessages()
{
    SCOPED_CPU_SAMPLE(processInputMessages);

    // Handle any dropped messages.
    processMessageDropEvents();

    // Get the queue reference for this tick's messages.
    std::queue<std::unique_ptr<ClientInput>>& messageQueue
        = network.startReceiveInputMessages(sim.getCurrentTick());

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
        auto clientEntityIt = world.netIdMap.find(inputMessage->netID);

        // Update the client entity's inputs.
        if (clientEntityIt != world.netIdMap.end()) {
            // Update the entity's Input component.
            entt::entity clientEntity = clientEntityIt->second;
            Input& input = world.registry.get<Input>(clientEntity);
            input = inputMessage->input;

            // Flag the entity as dirty.
            // It might already be dirty from a drop, so check first.
            if (!(world.registry.has<IsDirty>(clientEntity))) {
                world.registry.emplace<IsDirty>(clientEntity);
            }
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
    // Process any message drop events.
    for (unsigned int i = 0; i < messageDroppedQueue.size(); ++i) {
        // Pop the NetworkID of the client that we dropped a message from.
        ClientMessageDropped messageDropped{};
        if (messageDroppedQueue.pop(messageDropped)) {
            // Find the EntityID associated with the popped NetworkID.
            auto clientEntityIt = world.netIdMap.find(messageDropped.clientID);
            if (clientEntityIt != world.netIdMap.end()) {
                // We found the entity that dropped the message, handle it.
                entt::entity clientEntity = clientEntityIt->second;
                handleDropForEntity(clientEntity);
            }
            else {
                LOG_ERROR("Failed to find entity with netID: %u while "
                          "processing a message drop event.",
                          messageDropped.clientID);
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
    entt::registry& registry = world.registry;
    Input& entityInput = registry.get<Input>(entity);

    // Default the entity's inputs so they don't run off a cliff.
    Input defaultInput{};
    if (entityInput.inputStates != defaultInput.inputStates) {
        entityInput.inputStates = defaultInput.inputStates;

        // Flag the entity as dirty.
        world.registry.emplace<IsDirty>(entity);
    }

    // Flag that a drop occurred for this entity.
    registry.get<ClientSimData>(entity).messageWasDropped = true;
}

} // namespace Server
} // namespace AM
