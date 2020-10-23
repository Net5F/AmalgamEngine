#include "WorldSim.h"
#include "Network.h"
#include "ClientNetworkDefs.h"
#include "MessageTools.h"
#include "ConnectionResponse.h"
#include "ClientInputs.h"
#include "Peer.h"
#include "Log.h"
#include <memory>
#include <algorithm>

namespace AM
{
namespace LTC
{

/** An unreasonable amount of time for the game tick to be late by. */
static constexpr double GAME_DELAYED_TIME_S = .001;

WorldSim::WorldSim(Client::Network& inNetwork)
: network(inNetwork)
, entityID(0)
, accumulatedTime(0.0)
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
    std::unique_ptr<ConnectionResponse> connectionResponse
        = network.receiveConnectionResponse(CONNECTION_RESPONSE_WAIT_MS);
    if (connectionResponse == nullptr) {
        LOG_ERROR("Server did not respond.");
    }

    // Get our info from the connection response.
    entityID = connectionResponse->entityID;
    LOG_INFO("Received connection response. ID: %u, tick: %u", entityID,
             connectionResponse->tickNum);

    // Aim our tick for some reasonable point ahead of the server.
    // The server will adjust us after the first message anyway.
    currentTick = connectionResponse->tickNum + Client::INITIAL_TICK_OFFSET;
}

void WorldSim::tick()
{
    accumulatedTime += iterationTimer.getDeltaSeconds(true);

    // Process as many game ticks as have accumulated.
    while (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
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
            std::shared_ptr<const EntityUpdate> entityUpdate =
                network.receivePlayerUpdate(0);
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

        accumulatedTime -= GAME_TICK_TIMESTEP_S;
        if (accumulatedTime >= GAME_TICK_TIMESTEP_S) {
            LOG_INFO("Thread %u: Detected a request for multiple game ticks in the same "
                     "frame. Game tick "
                     "must have been massively delayed. Game tick was delayed "
                     "by: %.8fs.", entityID, accumulatedTime);
        }
        else if (accumulatedTime >= GAME_DELAYED_TIME_S) {
            // Game missed its ideal call time, could be our issue or general
            // system slowness.
            LOG_INFO("Thread %u: Detected a delayed game tick. Game tick was delayed by: "
                     "%.8fs.", entityID, accumulatedTime);
        }

        // Check our execution time.
        double executionTime = iterationTimer.getDeltaSeconds(false);
        if (executionTime > GAME_TICK_TIMESTEP_S) {
            LOG_INFO("Thread %u: Overran our sim iteration time. executionTime: %.8f",
                     entityID, executionTime);
        }
    }
}

void WorldSim::initTimer()
{
    iterationTimer.updateSavedTime();
}

void WorldSim::sendNextInput()
{
    // Construct the next input.
    ClientInputs clientInputs{};
    clientInputs.tickNum = currentTick;

    if (isMovingRight) {
        clientInputs.inputComponent.inputStates[Input::Left] = Input::Pressed;
        isMovingRight = false;
    }
    else {
        clientInputs.inputComponent.inputStates[Input::Right] = Input::Pressed;
        isMovingRight = true;
    }

    // Serialize the client inputs message.
    BinaryBufferSharedPtr messageBuffer
        = std::make_shared<BinaryBuffer>(Peer::MAX_MESSAGE_SIZE);
    unsigned int startIndex = CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE;
    std::size_t messageSize
        = MessageTools::serialize(*messageBuffer, clientInputs, startIndex);

    // Fill the buffer with the appropriate message header.
    MessageTools::fillMessageHeader(MessageType::ClientInputs, messageSize,
                                    messageBuffer, CLIENT_HEADER_SIZE);

    // Send the message.
    network.send(messageBuffer);
}

} // End namespace LTC
} // End namespace AM
