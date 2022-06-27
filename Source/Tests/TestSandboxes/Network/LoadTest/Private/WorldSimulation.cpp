#include "WorldSimulation.h"
#include "Network.h"
#include "Config.h"
#include "Serialize.h"
#include "InputChangeRequest.h"
#include "Peer.h"
#include "Log.h"
#include <memory>
#include <algorithm>

namespace AM
{
namespace LTC
{
WorldSimulation::WorldSimulation(EventDispatcher& inNetworkEventDispatcher,
                                 Client::Network& inNetwork)
: network(inNetwork)
, connectionResponseQueue(inNetworkEventDispatcher)
, clientEntity(entt::null)
, currentTick(0)
, ticksTillInput(0)
, isMovingRight(false)
{
    network.registerCurrentTickPtr(&currentTick);
}

void WorldSimulation::connect()
{
    while (!(network.connect())) {
        LOG_INFO("Network failed to connect. Retrying.");
    }

    // Wait for the player's ID from the server.
    ConnectionResponse connectionResponse{};
    ;
    if (!(connectionResponseQueue.waitPop(connectionResponse,
                                          CONNECTION_RESPONSE_WAIT_US))) {
        LOG_FATAL("Server did not respond.");
    }

    // Get our info from the connection response.
    clientEntity = connectionResponse.entity;
    LOG_INFO("Received connection response. ID: %u, tick: %u", clientEntity,
             connectionResponse.tickNum);

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick
        = connectionResponse.tickNum + Client::Config::INITIAL_TICK_OFFSET;
}

void WorldSimulation::tick()
{
    Uint32 targetTick{currentTick + 1};

    // Apply any adjustments that we receive from the server.
    targetTick += network.transferTickAdjustment();

    // Process ticks until we match what the server wants.
    // This may cause us to not process any ticks, or to process multiple
    // ticks.
    while (currentTick < targetTick) {
        // If it's time to move, send an input message.
        if (ticksTillInput == 0) {
            sendNextInput();
            ticksTillInput = INPUT_RATE_TICKS;
        }
        ticksTillInput--;

        currentTick++;
    }
}

void WorldSimulation::sendNextInput()
{
    // Construct the next input.
    InputChangeRequest inputChangeRequest{};
    inputChangeRequest.tickNum = currentTick;

    if (isMovingRight) {
        inputChangeRequest.input.inputStates[Input::XUp] = Input::Pressed;
        isMovingRight = false;
    }
    else {
        inputChangeRequest.input.inputStates[Input::XDown] = Input::Pressed;
        isMovingRight = true;
    }

    // Send the client input message.
    network.serializeAndSend<InputChangeRequest>(inputChangeRequest);
}

} // End namespace LTC
} // End namespace AM
