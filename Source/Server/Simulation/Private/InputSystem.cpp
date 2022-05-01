#include "InputSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "Input.h"
#include "InputHasChanged.h"
#include "ClientSimData.h"
#include "Log.h"
#include <memory>
#include "Tracy.hpp"

namespace AM
{
namespace Server
{
InputSystem::InputSystem(Simulation& inSim, World& inWorld,
                         EventDispatcher& inNetworkEventDispatcher,
                         Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
, inputChangeRequestQueue(inNetworkEventDispatcher)
{
}

void InputSystem::processInputMessages()
{
    ZoneScoped;

    // Sort any waiting client input events.
    while (InputChangeRequest* inputChangeRequest
           = inputChangeRequestQueue.peek()) {
        // Push the event into the sorter.
        SorterBase::ValidityResult result = inputChangeRequestSorter.push(
            *inputChangeRequest, inputChangeRequest->tickNum);

        // If we had to drop an event, handle it.
        if (result != SorterBase::ValidityResult::Valid) {
            handleDroppedMessage(inputChangeRequest->netID);
        }

        inputChangeRequestQueue.pop();
    }

    // Process all client input events for this tick.
    std::queue<InputChangeRequest>* queue
        = inputChangeRequestSorter.getCurrentQueue();
    while (!(queue->empty())) {
        // Get the next event.
        InputChangeRequest& inputChangeRequest = queue->front();

        // If the input is from an earlier tick, drop it and continue.
        if (inputChangeRequest.tickNum < sim.getCurrentTick()) {
            LOG_INFO("Dropped message from %u. Tick: %u, received: %u",
                     inputChangeRequest.netID, sim.getCurrentTick(),
                     inputChangeRequest.tickNum);
            handleDroppedMessage(inputChangeRequest.netID);
            inputChangeRequestQueue.pop();
            continue;
        }
        // If the message is from a later tick, we're done.
        else if (inputChangeRequest.tickNum > sim.getCurrentTick()) {
            break;
        }

        // Find the entity associated with the given NetID.
        auto clientEntityIt = world.netIdMap.find(inputChangeRequest.netID);

        // Update the client entity's inputs.
        if (clientEntityIt != world.netIdMap.end()) {
            // Update the entity's Input component.
            entt::entity clientEntity = clientEntityIt->second;
            Input& input = world.registry.get<Input>(clientEntity);
            input = inputChangeRequest.input;

            // Tag the entity as dirty.
            // It might already be dirty from a drop, so check first.
            if (!(world.registry.all_of<InputHasChanged>(clientEntity))) {
                world.registry.emplace<InputHasChanged>(clientEntity);
            }
        }
        else {
            // The entity was probably disconnected. Do nothing with the
            // message.
        }

        queue->pop();
    }

    // Advance the sorter to the next tick.
    inputChangeRequestSorter.advance();
}

void InputSystem::handleDroppedMessage(NetworkID clientID)
{
    // Find the entity ID of the client that we dropped a message from.
    auto clientEntityIt = world.netIdMap.find(clientID);
    if (clientEntityIt == world.netIdMap.end()) {
        // TODO: Think about this. It may be a normal thing that we should just
        //       return early from instead of erroring.
        LOG_FATAL("Failed to find entity with netID: %u while "
                  "processing a message drop event.",
                  clientID);
    }

    entt::registry& registry = world.registry;
    Input& entityInput = registry.get<Input>(clientEntityIt->second);

    // Default the entity's inputs so they don't run off a cliff.
    Input defaultInput{};
    if (entityInput.inputStates != defaultInput.inputStates) {
        entityInput.inputStates = defaultInput.inputStates;

        // Flag the entity as dirty.
        registry.emplace<InputHasChanged>(clientEntityIt->second);
    }

    // Flag that a drop occurred for this entity.
    registry.get<ClientSimData>(clientEntityIt->second).inputWasDropped = true;
}

} // namespace Server
} // namespace AM
