#include "WorldSim.h"
#include "Network.h"
#include "ClientNetworkDefs.h"
#include "Config.h"
#include "Serialize.h"
#include "ConnectionResponse.h"
#include "ClientInput.h"
#include "Peer.h"
#include "Log.h"
#include <memory>
#include <algorithm>

namespace AM
{
namespace LTC
{
WorldSim::WorldSim(Client::Network& inNetwork)
: network(inNetwork)
, clientEntity(entt::null)
, currentTick(0)
, ticksTillInput(0)
, isMovingRight(false)
{
    network.registerCurrentTickPtr(&currentTick);
}

void WorldSim::connect()
{
    while (!(network.connect())) {
        LOG_INFO("Network failed to connect. Retrying.");
    }

    // Wait for the player's ID from the server.
    std::shared_ptr<ConnectionResponse> connectionResponse
        = network.receiveConnectionResponse(CONNECTION_RESPONSE_WAIT_MS);
    if (connectionResponse == nullptr) {
        LOG_ERROR("Server did not respond.");
    }

    // Get our info from the connection response.
    clientEntity = connectionResponse->entity;
    LOG_INFO("Received connection response. ID: %u, tick: %u", clientEntity,
             connectionResponse->tickNum);

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick
        = connectionResponse->tickNum + Client::Config::INITIAL_TICK_OFFSET;
}

void WorldSim::tick()
{
    Uint32 targetTick = currentTick + 1;

    // Apply any adjustments that we receive from the server.
    targetTick += network.transferTickAdjustment();

    /* Process ticks until we match what the server wants.
       This may cause us to not process any ticks, or to process multiple
       ticks. */
    while (currentTick < targetTick) {
        // If it's time to move, send an input message.
        if (ticksTillInput == 0) {
            sendNextInput();
            ticksTillInput = INPUT_RATE_TICKS;
        }
        ticksTillInput--;

        // Receive any waiting player messages.
        std::shared_ptr<const EntityUpdate> entityUpdate
            = network.receivePlayerUpdate(0);
        while (entityUpdate != nullptr) {
            entityUpdate = network.receivePlayerUpdate(0);
        }

        // Receive any waiting npc messages.
        Client::NpcReceiveResult receiveResult = network.receiveNpcUpdate();
        while (receiveResult.result == NetworkResult::Success) {
            receiveResult = network.receiveNpcUpdate();
        }

        currentTick++;
    }
}

void WorldSim::sendNextInput()
{
    // Construct the next input.
    ClientInput clientInput{};
    clientInput.tickNum = currentTick;

    if (isMovingRight) {
        clientInput.input.inputStates[Input::XUp] = Input::Pressed;
        isMovingRight = false;
    }
    else {
        clientInput.input.inputStates[Input::XDown] = Input::Pressed;
        isMovingRight = true;
    }

    // Send the client input message.
    network.serializeAndSend<ClientInput>(clientInput);
}

} // End namespace LTC
} // End namespace AM
