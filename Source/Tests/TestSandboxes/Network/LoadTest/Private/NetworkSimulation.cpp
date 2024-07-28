#include "NetworkSimulation.h"
#include "Heartbeat.h"
#include "ConnectionError.h"
#include "Config.h"
#include "UserConfig.h"
#include "AMAssert.h"
#include <SDL_net.h>

namespace AM
{
namespace LTC
{
NetworkSimulation::NetworkSimulation()
: server{nullptr}
, serverConnected{false}
, eventDispatcher{}
, messageProcessor{eventDispatcher}
, tickAdjustment{0}
, adjustmentIteration{0}
, isApplyingTickAdjustment{false}
, messagesSentSinceTick{0}
, currentTickPtr{nullptr}
, headerRecBuffer(SERVER_HEADER_SIZE)
, batchRecBuffer(SharedConfig::MAX_BATCH_SIZE)
, decompressedBatchRecBuffer(SharedConfig::MAX_BATCH_SIZE)
{
}

void NetworkSimulation::connect()
{
    if (server != nullptr) {
        LOG_INFO("Attempted to connect while connected.");
        return;
    }

    // Try to connect.
    Client::ServerAddress serverAddress{
        Client::UserConfig::get().getServerAddress()};
    server = Peer::initiate(serverAddress.IP, serverAddress.port);
    if (server != nullptr) {
        // Note: The server sends us a ConnectionResponse when we connect the
        //       socket. Eventually, we'll instead send a ConnectionRequest to
        //       the login server here.

        serverConnected = true;
    }
    else {
        eventDispatcher.emplace<Client::ConnectionError>(
            Client::ConnectionError::Type::Failed);
        return;
    }
}

void NetworkSimulation::disconnect()
{
    server = nullptr;
    serverConnected = false;
    adjustmentIteration = 0;
    isApplyingTickAdjustment = false;
    messagesSentSinceTick = 0;
}

void NetworkSimulation::tick()
{
    // If the server connection isn't established, do nothing (we don't want 
    // to try to read the server var while connect() is potentially running 
    // on another thread).
    if (!serverConnected) {
        return;
    }

    // Receive any waiting messages for this client.
    receiveAndProcess();

    // Send a heartbeat if we need to.
    if (*currentTickPtr != 0) {
        sendHeartbeatIfNecessary();
    }
}

void NetworkSimulation::receiveAndProcess()
{
    // If the server connection isn't established, do nothing (we don't want 
    // to try to read the server var while connect() is potentially running 
    // on another thread).
    if (!serverConnected) {
        return;
    }

    // Receive message batches from the server.
    NetworkResult headerResult{
        server->receiveBytes(headerRecBuffer.data(), SERVER_HEADER_SIZE, true)};
    while (headerResult != NetworkResult::NoWaitingData) {
        switch (headerResult) {
            case NetworkResult::Success: {
                processBatch();
                break;
            }
            case NetworkResult::Disconnected: {
                LOG_INFO("Found server to be disconnected while trying to "
                         "receive header.");
                eventDispatcher.emplace<Client::ConnectionError>(
                    Client::ConnectionError::Type::Disconnected);
                return;
            }
            default: {
                break;
            }
        }

        headerResult = server->receiveBytes(headerRecBuffer.data(),
                                            SERVER_HEADER_SIZE, true);
    }
}

EventDispatcher& NetworkSimulation::getEventDispatcher()
{
    return eventDispatcher;
}

Uint32 NetworkSimulation::getLastReceivedTick()
{
    return messageProcessor.getLastReceivedTick();
}

int NetworkSimulation::transferTickAdjustment()
{
    if (isApplyingTickAdjustment) {
        int currentAdjustment = tickAdjustment;
        if (currentAdjustment < 0) {
            // The sim can only freeze for 1 tick at a time, transfer 1 from
            // tickAdjustment.
            tickAdjustment += 1;
            return -1;
        }
        else if (currentAdjustment > 0) {
            // The sim can process multiple iterations to catch up, transfer
            // all of tickAdjustment.
            tickAdjustment -= currentAdjustment;
            return currentAdjustment;
        }
        else {
            // We finished applying the adjustment, increment the iteration.
            adjustmentIteration++;
            isApplyingTickAdjustment = false;
            return 0;
        }
    }
    else {
        return 0;
    }
}

void NetworkSimulation::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void NetworkSimulation::send(const BinaryBufferSharedPtr& message)
{
    if ((server == nullptr) || !(server->isConnected())) {
        // Note: Receive thread is responsible for emitting ConnectionError.
        LOG_INFO("Tried to send while server is disconnected.");
        return;
    }

    // Send the message.
    NetworkResult result{server->send(message)};
    if (result == NetworkResult::Success) {
        messagesSentSinceTick++;
    }
    else {
        // Note: Receive thread is responsible for emitting ConnectionError.
        LOG_INFO("Message send failed.");
    }
}

void NetworkSimulation::sendHeartbeatIfNecessary()
{
    // If we haven't sent any relevant messages since the last tick.
    if (messagesSentSinceTick == 0) {
        // Send the heartbeat message.
        serializeAndSend<Heartbeat>({*currentTickPtr});
    }

    messagesSentSinceTick = 0;
}

void NetworkSimulation::processBatch()
{
    // Check if we need to adjust the tick offset.
    adjustIfNeeded(headerRecBuffer[ServerHeaderIndex::TickAdjustment],
                   headerRecBuffer[ServerHeaderIndex::AdjustmentIteration]);

    /* Process the BatchSize header field. */
    // Read the high bit of batchSize to tell whether the batch is compressed
    // or not. If the high bit is set, the batch is compressed.
    Uint16 batchSize{
        ByteTools::read16(&(headerRecBuffer[ServerHeaderIndex::BatchSize]))};
    bool batchIsCompressed{(batchSize & (1U << 15)) != 0};

    // Reset the high bit of batchSize to get the real size.
    batchSize &= ~(1U << 15);

    /* Process the batch, if it contains any data. */
    if (batchSize > 0) {
        // Receive the expected bytes.
        NetworkResult result{
            server->receiveBytesWait(&(batchRecBuffer[0]), batchSize)};
        if (result != NetworkResult::Success) {
            LOG_INFO("Failed to receive expected bytes.");
        }

        // If the payload is compressed, decompress it.
        Uint8* bufferToUse{&(batchRecBuffer[0])};
        if (batchIsCompressed) {
            batchSize = static_cast<Uint16>(
                ByteTools::decompress(&(batchRecBuffer[0]), batchSize,
                                      &(decompressedBatchRecBuffer[0]),
                                      SharedConfig::MAX_BATCH_SIZE));

            bufferToUse = &(decompressedBatchRecBuffer[0]);
        }

        // Process the messages.
        std::size_t bufferIndex{0};
        while (bufferIndex < batchSize) {
            Uint8 messageType{
                bufferToUse[bufferIndex + MessageHeaderIndex::MessageType]};
            Uint16 messageSize{ByteTools::read16(
                &(bufferToUse[bufferIndex + MessageHeaderIndex::Size]))};

            messageProcessor.processReceivedMessage(
                messageType,
                &(bufferToUse[bufferIndex + MessageHeaderIndex::MessageStart]),
                messageSize);

            bufferIndex += MESSAGE_HEADER_SIZE + messageSize;
            AM_ASSERT((bufferIndex <= batchSize),
                      "Buffer index is wrong. %u, %u", bufferIndex, batchSize);
        }

        AM_ASSERT((bufferIndex == batchSize),
                  "Didn't process correct number of bytes. %u, %u", bufferIndex,
                  batchSize);
    }
}

void NetworkSimulation::adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration)
{
    if (receivedTickAdj != 0) {
        Uint8 currentAdjIteration{adjustmentIteration};

        // If it's the current iteration and we aren't already applying it.
        if ((receivedAdjIteration == currentAdjIteration)
            && !isApplyingTickAdjustment) {
            // Set the adjustment to be applied.
            tickAdjustment += receivedTickAdj;
            isApplyingTickAdjustment = true;
            //LOG_INFO("Received tick adjustment: %d, iteration: %u",
            //         receivedTickAdj, receivedAdjIteration);
        }
        else if (receivedAdjIteration > currentAdjIteration) {
            if (isApplyingTickAdjustment) {
                LOG_FATAL("Received future adjustment iteration while applying "
                          "the last. current: %u, received: %u",
                          currentAdjIteration, receivedAdjIteration);
            }
            else {
                LOG_FATAL("Out of sequence adjustment iteration. current: %u, "
                          "received: %u",
                          currentAdjIteration, receivedAdjIteration);
            }
        }
    }
}

} // End namespace LTC
} // namespace AM
