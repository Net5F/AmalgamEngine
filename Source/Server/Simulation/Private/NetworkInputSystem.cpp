#include "NetworkInputSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
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
, clientInputQueue(inNetwork.getDispatcher())
{
}

void NetworkInputSystem::processInputMessages()
{
    SCOPED_CPU_SAMPLE(processInputMessages);

    // Process any client input messages.
    while (ClientInput* clientInput = clientInputQueue.peek()) {
        // If the input is from an earlier tick, drop it and continue.
        if (clientInput->tickNum < sim.getCurrentTick()) {
            LOG_INFO("Dropped message from %u. Tick: %u, received: %u"
                , clientInput->netID, sim.getCurrentTick(), clientInput->tickNum);
            handleDroppedMessage(clientInput->netID);
            clientInputQueue.pop();
            continue;
        }
        // If the message is from a later tick, we're done.
        else if (clientInput->tickNum > sim.getCurrentTick()) {
            break;
        }

        // Find the entity associated with the given NetID.
        auto clientEntityIt = world.netIdMap.find(clientInput->netID);

        // Update the client entity's inputs.
        if (clientEntityIt != world.netIdMap.end()) {
            // Update the entity's Input component.
            entt::entity clientEntity = clientEntityIt->second;
            Input& input = world.registry.get<Input>(clientEntity);
            input = clientInput->input;

            // Flag the entity as dirty.
            // It might already be dirty from a drop, so check first.
            if (!(world.registry.all_of<IsDirty>(clientEntity))) {
                world.registry.emplace<IsDirty>(clientEntity);
            }
        }
        else {
            // The entity was probably disconnected. Do nothing with the
            // message.
        }

        clientInputQueue.pop();
    }
}

void NetworkInputSystem::handleDroppedMessage(NetworkID clientID)
{
    // Find the entity ID of the client that we dropped a message from.
    auto clientEntityIt = world.netIdMap.find(clientID);
    if (clientEntityIt == world.netIdMap.end()) {
        // TODO: Think about this. It may be a normal thing that we should just
        //       return early from instead of erroring.
        LOG_ERROR("Failed to find entity with netID: %u while "
                  "processing a message drop event.", clientID);
    }

    entt::registry& registry = world.registry;
    Input& entityInput = registry.get<Input>(clientEntityIt->second);

    // Default the entity's inputs so they don't run off a cliff.
    Input defaultInput{};
    if (entityInput.inputStates != defaultInput.inputStates) {
        entityInput.inputStates = defaultInput.inputStates;

        // Flag the entity as dirty.
        registry.emplace<IsDirty>(clientEntityIt->second);
    }

    // Flag that a drop occurred for this entity.
    registry.get<ClientSimData>(clientEntityIt->second).messageWasDropped = true;
}

} // namespace Server
} // namespace AM
