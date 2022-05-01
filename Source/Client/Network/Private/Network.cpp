#include "Network.h"
#include "QueuedEvents.h"
#include "Heartbeat.h"
#include "Config.h"
#include "NetworkStats.h"
#include "AMAssert.h"
#include "Ignore.h"
#include <SDL_net.h>

namespace AM
{
namespace Client
{
Network::Network()
: server(nullptr)
, messageProcessor(eventDispatcher)
, eventDispatcher()
, tickAdjustment(0)
, adjustmentIteration(0)
, isApplyingTickAdjustment(false)
, messagesSentSinceTick(0)
, currentTickPtr(nullptr)
, receiveThreadObj()
, exitRequested(false)
, headerRecBuffer(SERVER_HEADER_SIZE)
, batchRecBuffer(Peer::MAX_WIRE_SIZE)
, uncompressedBatchRecBuffer(SharedConfig::MAX_BATCH_SIZE)
, netstatsLoggingEnabled(true)
, ticksSinceNetstatsLog(0)
{
    if (!Config::RUN_OFFLINE) {
        SDLNet_Init();
    }

    // Init the timer to the current time.
    receiveTimer.updateSavedTime();
}

Network::~Network()
{
    exitRequested = true;

    if (!Config::RUN_OFFLINE) {
        receiveThreadObj.join();
        SDLNet_Quit();
    }
}

bool Network::connect()
{
    // Try to connect.
    server = Peer::initiate(Config::SERVER_IP, Config::SERVER_PORT);

    // Spin up the receive thread.
    if (server != nullptr) {
        receiveThreadObj = std::thread(&Network::pollForMessages, this);
    }

    return (server != nullptr);
}

void Network::tick()
{
    if (!Config::RUN_OFFLINE) {
        // Send a heartbeat if we need to.
        sendHeartbeatIfNecessary();

        // If it's time to log our network statistics, do so.
        if (netstatsLoggingEnabled) {
            ticksSinceNetstatsLog++;
            if (ticksSinceNetstatsLog == TICKS_TILL_STATS_DUMP) {
                logNetworkStatistics();
                ticksSinceNetstatsLog = 0;
            }
        }
    }
}

EventDispatcher& Network::getEventDispatcher()
{
    return eventDispatcher;
}

int Network::transferTickAdjustment()
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

void Network::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Network::setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled)
{
    netstatsLoggingEnabled = inNetstatsLoggingEnabled;
}

void Network::send(const BinaryBufferSharedPtr& message)
{
    if ((server == nullptr) || !(server->isConnected())) {
        LOG_FATAL("Tried to send while server is disconnected.");
    }

    // Send the message.
    NetworkResult result{server->send(message)};
    if (result == NetworkResult::Success) {
        messagesSentSinceTick++;

        // Record the number of sent bytes.
        NetworkStats::recordBytesSent(
            static_cast<unsigned int>(message->size()));
    }
    else {
        LOG_FATAL("Message send failed.");
    }
}

void Network::sendHeartbeatIfNecessary()
{
    // If we haven't sent any relevant messages since the last tick.
    if (messagesSentSinceTick == 0) {
        // Send the heartbeat message.
        serializeAndSend<Heartbeat>({*currentTickPtr});
    }

    messagesSentSinceTick = 0;
}

int Network::pollForMessages()
{
    while (!exitRequested) {
        // Wait for a server header.
        NetworkResult headerResult{server->receiveBytesWait(
            headerRecBuffer.data(), SERVER_HEADER_SIZE)};

        if (headerResult == NetworkResult::Success) {
            processBatch();
        }
        else if (headerResult == NetworkResult::Disconnected) {
            LOG_FATAL("Found server to be disconnected while trying to "
                      "receive header.");
        }
    }

    return 0;
}

void Network::processBatch()
{
    // Start tracking the number of received bytes.
    unsigned int bytesReceived{SERVER_HEADER_SIZE};

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
        AM_ASSERT((result == NetworkResult::Success),
                  "Failed to receive expected bytes.");
        ignore(result);

        // Track the number of bytes we've received.
        bytesReceived += MESSAGE_HEADER_SIZE + batchSize;

        // If the payload is compressed, uncompress it.
        Uint8* bufferToUse{&(batchRecBuffer[0])};
        if (batchIsCompressed) {
            batchSize = static_cast<Uint16>(
                ByteTools::uncompress(&(batchRecBuffer[0]), batchSize,
                                      &(uncompressedBatchRecBuffer[0]),
                                      SharedConfig::MAX_BATCH_SIZE));

            bufferToUse = &(uncompressedBatchRecBuffer[0]);
        }

        // Process the messages.
        std::size_t bufferIndex{0};
        while (bufferIndex < batchSize) {
            MessageType messageType{static_cast<MessageType>(
                bufferToUse[bufferIndex + MessageHeaderIndex::MessageType])};
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

    // Record the number of received bytes.
    NetworkStats::recordBytesReceived(bytesReceived);
}

void Network::adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration)
{
    if (receivedTickAdj != 0) {
        Uint8 currentAdjIteration{adjustmentIteration};

        // If it's the current iteration and we aren't already applying it.
        if ((receivedAdjIteration == currentAdjIteration)
            && !isApplyingTickAdjustment) {
            // Set the adjustment to be applied.
            tickAdjustment += receivedTickAdj;
            isApplyingTickAdjustment = true;
            LOG_INFO("Received tick adjustment: %d, iteration: %u",
                     receivedTickAdj, receivedAdjIteration);
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

void Network::logNetworkStatistics()
{
    // Dump the stats from the tracker.
    NetStatsDump netStats{NetworkStats::dumpStats()};

    // Log the stats.
    float bytesSentPerSecond{netStats.bytesSent
                             / static_cast<float>(SECONDS_TILL_STATS_DUMP)};
    float bytesReceivedPerSecond{netStats.bytesReceived
                                 / static_cast<float>(SECONDS_TILL_STATS_DUMP)};
    LOG_INFO("Bytes sent per second: %.0f, Bytes received per second: %.0f",
             bytesSentPerSecond, bytesReceivedPerSecond);
}

} // namespace Client
} // namespace AM
