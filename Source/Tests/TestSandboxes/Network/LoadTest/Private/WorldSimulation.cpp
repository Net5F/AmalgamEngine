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
                                 Client::Network& inNetwork,
                                 unsigned int inInputsPerSecond)
: network{inNetwork}
, connectionResponseQueue{inNetworkEventDispatcher}
, connectionErrorQueue{inNetworkEventDispatcher}
, clientEntity{entt::null}
, currentTick{0}
, inputsPerSecond{inInputsPerSecond}
, ticksTillInput{0}
, isMovingRight{false}
{
    network.registerCurrentTickPtr(&currentTick);
}

void WorldSimulation::connect()
{
    network.connect();

    // Wait for the player's ID from the server.
    ConnectionResponse connectionResponse{};
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
    // First, make sure we still have a connection.
    Client::ConnectionError connectionError;
    if (connectionErrorQueue.pop(connectionError)) {
        LOG_FATAL("Lost connection to server.");
    }

    Uint32 targetTick{currentTick + 1};

    // Apply any adjustments that we received from the server.
    targetTick += network.transferTickAdjustment();

    // Process ticks until we match what the server wants.
    // This may cause us to not process any ticks, or to process multiple
    // ticks.
    while (currentTick < targetTick) {
        if (inputsPerSecond > 0) {
            // If it's time to move, send an input message.
            if (ticksTillInput == 0) {
                sendNextInput();
                ticksTillInput
                    = (SharedConfig::SIM_TICKS_PER_SECOND / inputsPerSecond);
            }
            ticksTillInput--;
        }

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
