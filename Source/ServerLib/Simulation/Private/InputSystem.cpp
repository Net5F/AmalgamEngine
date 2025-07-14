#include "InputSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "Input.h"
#include "ClientSimData.h"
#include "Log.h"
#include "tracy/Tracy.hpp"
#include <memory>

namespace AM
{
namespace Server
{
InputSystem::InputSystem(Simulation& inSimulation, World& inWorld,
                         Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, inputChangeRequestQueue{inNetwork.getEventDispatcher()}
, inputChangeRequestSorter{}
{
}

void InputSystem::processInputMessages()
{
    ZoneScoped;

    // Sort any waiting client input requests.
    while (const InputChangeRequest
           * inputChangeRequest{inputChangeRequestQueue.peek()}) {
        // Push the request into the sorter.
        SorterBase::ValidityResult result{inputChangeRequestSorter.push(
            *inputChangeRequest, inputChangeRequest->tickNum)};

        // If we had to drop a request, handle it.
        if (result != SorterBase::ValidityResult::Valid) {
            LOG_INFO("Dropped message from %u. Tick: %u, received: %u",
                     inputChangeRequest->netID, simulation.getCurrentTick(),
                     inputChangeRequest->tickNum);
            handleDroppedMessage(inputChangeRequest->netID);
        }

        inputChangeRequestQueue.pop();
    }

    // Process all client input requests for this tick.
    std::queue<InputChangeRequest>& queue{
        inputChangeRequestSorter.getCurrentQueue()};
    Uint32 currentTick{simulation.getCurrentTick()};
    for (; !(queue.empty()); queue.pop()) {
        // Get the next request.
        InputChangeRequest& inputChangeRequest{queue.front()};

        // If the input is from an earlier tick, drop it and continue.
        if (inputChangeRequest.tickNum < currentTick) {
            LOG_INFO("Dropped message from %u. Tick: %u, received: %u",
                     inputChangeRequest.netID, currentTick,
                     inputChangeRequest.tickNum);
            handleDroppedMessage(inputChangeRequest.netID);
            inputChangeRequestQueue.pop();
            continue;
        }
        // If the message is from a later tick, we're done.
        else if (inputChangeRequest.tickNum > currentTick) {
            break;
        }

        // Find the entity associated with the given NetID.
        auto clientEntityIt{world.netIDMap.find(inputChangeRequest.netID)};

        // Update the client entity's inputs.
        if (clientEntityIt != world.netIDMap.end()) {
            // Update the entity's Input component.
            entt::entity clientEntity{clientEntityIt->second};
            world.registry.replace<Input>(clientEntity,
                                          inputChangeRequest.input);
        }
        else {
            // The entity was probably disconnected. Do nothing with the
            // message.
        }
    }

    // Advance the sorter to the next tick.
    inputChangeRequestSorter.advance();
}

void InputSystem::handleDroppedMessage(NetworkID clientID)
{
    // Find the entity ID of the client that we dropped a message from.
    auto clientEntityIt{world.netIDMap.find(clientID)};
    if (clientEntityIt == world.netIDMap.end()) {
        // The entity is gone, we don't need to process this drop.
        return;
    }

    entt::registry& registry{world.registry};
    Input& entityInput{registry.get<Input>(clientEntityIt->second)};

    // Default the entity's inputs so they don't run off a cliff.
    Input defaultInput{};
    if (entityInput.inputStates != defaultInput.inputStates) {
        registry.replace<Input>(clientEntityIt->second, defaultInput);
    }
}

} // namespace Server
} // namespace AM
