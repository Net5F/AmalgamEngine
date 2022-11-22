#include "InputSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "Input.h"
#include "MovementStateNeedsSync.h"
#include "ClientSimData.h"
#include "Log.h"
#include "Tracy.hpp"
#include <memory>

namespace AM
{
namespace Server
{
InputSystem::InputSystem(Simulation& inSimulation, World& inWorld,
                         EventDispatcher& inNetworkEventDispatcher)
: simulation{inSimulation}
, world{inWorld}
, inputChangeRequestQueue{inNetworkEventDispatcher}
{
}

void InputSystem::processInputMessages()
{
    ZoneScoped;

    // Sort any waiting client input events.
    while (InputChangeRequest* inputChangeRequest
           = inputChangeRequestQueue.peek()) {
        // Push the event into the sorter.
        SorterBase::ValidityResult result{inputChangeRequestSorter.push(
            *inputChangeRequest, inputChangeRequest->tickNum)};

        // If we had to drop an event, handle it.
        if (result != SorterBase::ValidityResult::Valid) {
            LOG_INFO("Dropped message from %u. Tick: %u, received: %u",
                     inputChangeRequest->netID, simulation.getCurrentTick(),
                     inputChangeRequest->tickNum);
            handleDroppedMessage(inputChangeRequest->netID);
        }

        inputChangeRequestQueue.pop();
    }

    // Process all client input events for this tick.
    std::queue<InputChangeRequest>& queue{
        inputChangeRequestSorter.getCurrentQueue()};
    while (!(queue.empty())) {
        // Get the next event.
        InputChangeRequest& inputChangeRequest{queue.front()};

        // If the input is from an earlier tick, drop it and continue.
        if (inputChangeRequest.tickNum < simulation.getCurrentTick()) {
            LOG_INFO("Dropped message from %u. Tick: %u, received: %u",
                     inputChangeRequest.netID, simulation.getCurrentTick(),
                     inputChangeRequest.tickNum);
            handleDroppedMessage(inputChangeRequest.netID);
            inputChangeRequestQueue.pop();
            continue;
        }
        // If the message is from a later tick, we're done.
        else if (inputChangeRequest.tickNum > simulation.getCurrentTick()) {
            break;
        }

        // Find the entity associated with the given NetID.
        auto clientEntityIt{world.netIdMap.find(inputChangeRequest.netID)};

        // Update the client entity's inputs.
        if (clientEntityIt != world.netIdMap.end()) {
            // Update the entity's Input component.
            entt::entity clientEntity{clientEntityIt->second};
            Input& input{world.registry.get<Input>(clientEntity)};
            input = inputChangeRequest.input;

            // Flag that the entity's movement state needs to be synced.
            if (!(world.registry.all_of<MovementStateNeedsSync>(
                    clientEntity))) {
                world.registry.emplace<MovementStateNeedsSync>(clientEntity);
            }
        }
        else {
            // The entity was probably disconnected. Do nothing with the
            // message.
        }

        queue.pop();
    }

    // Advance the sorter to the next tick.
    inputChangeRequestSorter.advance();
}

void InputSystem::handleDroppedMessage(NetworkID clientID)
{
    // Find the entity ID of the client that we dropped a message from.
    auto clientEntityIt{world.netIdMap.find(clientID)};
    if (clientEntityIt == world.netIdMap.end()) {
        // The entity is gone, we don't need to process this drop.
        return;
    }

    entt::registry& registry{world.registry};
    Input& entityInput{registry.get<Input>(clientEntityIt->second)};

    // Default the entity's inputs so they don't run off a cliff.
    Input defaultInput{};
    if (entityInput.inputStates != defaultInput.inputStates) {
        entityInput.inputStates = defaultInput.inputStates;
    }

    // Flag that the entity's movement state needs to be synced.
    if (!(world.registry.all_of<MovementStateNeedsSync>(
            clientEntityIt->second))) {
        registry.emplace<MovementStateNeedsSync>(clientEntityIt->second);
    }
}

} // namespace Server
} // namespace AM
