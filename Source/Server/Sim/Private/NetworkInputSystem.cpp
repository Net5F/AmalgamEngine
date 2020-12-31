#include "NetworkInputSystem.h"
#include "Sim.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "SimDefs.h"
#include "ClientInput.h"
#include "Input.h"
#include "IsDirty.h"
#include "ClientSimData.h"
#include "Log.h"
#include <memory>

namespace AM
{
namespace Server
{
NetworkInputSystem::NetworkInputSystem(Sim& inSim, World& inWorld,
                                       Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
{
}

Timer timer3;
double time3[4]{};
double tempTime3[4]{};
int timerCounter3 = 0;
void NetworkInputSystem::processInputMessages()
{
    // Handle any dropped messages.
    processMessageDropEvents();

    // Get the queue reference for this tick's messages.
    std::queue<std::unique_ptr<ClientInput>>& messageQueue
        = network.startReceiveInputMessages(sim.getCurrentTick());

    // Process all messages.
    while (!(messageQueue.empty())) {
        timer3.updateSavedTime();
        std::unique_ptr<ClientInput> inputMessage
            = std::move(messageQueue.front());
        messageQueue.pop();
        if (inputMessage == nullptr) {
            LOG_INFO("Failed to receive input message after getting count "
                     "(this shouldn't happen).")
        }
        tempTime3[0] += timer3.getDeltaSeconds(true);

        // Find the entity associated with the given NetID.
        auto clientEntityIt = world.netIdMap.find(inputMessage->netID);
        tempTime3[1] += timer3.getDeltaSeconds(true);

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
        tempTime3[2] += timer3.getDeltaSeconds(true);
    }

    network.endReceiveInputMessages();

    for (unsigned int i = 0; i < 4; ++i) {
        if (tempTime3[i] > time3[i]) {
            time3[i] = tempTime3[i];
        }
        tempTime3[i] = 0;
    }

    if (timerCounter3 == 150) {
        LOG_INFO("Pop: %.6f, Find: %.6f, Construct: %.6f", time3[0], time3[1], time3[2]);
        for (unsigned int i = 0; i < 4; ++i) {
            time3[i] = 0;
        }
        timerCounter3 = 0;
    }
    else {
        timerCounter3++;
    }
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
            auto clientEntityIt = world.netIdMap.find(clientNetworkID);
            if (clientEntityIt != world.netIdMap.end()) {
                // We found the entity that dropped the message, handle it.
                entt::entity clientEntity = clientEntityIt->second;
                handleDropForEntity(clientEntity);
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
