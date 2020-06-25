#ifndef NETWORKDEFS_H
#define NETWORKDEFS_H

#include <SDL_stdinc.h>
#include <memory>
#include <vector>

/**
 * This file contains shared network definitions that should be
 * consistent between the server and client.
 */
namespace AM
{
/** 20 network ticks per second. */
static constexpr float NETWORK_TICK_INTERVAL_S = 1 / 20.0f;

/** Represents a single network client. Will be reused if the client disconnects. */
typedef Uint32 NetworkID;

/** Dynamically allocated, portable buffers for messages. */
typedef std::vector<Uint8> BinaryBuffer;
typedef std::unique_ptr<BinaryBuffer> BinaryBufferPtr;
typedef std::shared_ptr<BinaryBuffer> BinaryBufferSharedPtr;


/** Used for indexing into the parts of a server header. */
enum ServerHeaderIndex : Uint8 {
    /** Sint8, the adjustment that the server wants the client to make. */
    TickOffsetAdjustment = 0,
    /** Uint8, the iteration of tick offset adjustment that we're on. */
    AdjustmentIteration = 1,
    /** Uint8, the number of messages in this batch. */
    MessageCount = 2
};
/** The size of a server header in bytes. */
static constexpr unsigned int SERVER_HEADER_SIZE = 3;

/** Used for indexing into the size or payload of a received message. */
enum MessageIndex : Uint8 {
    // Uint16
    Size = 0,
    // Uint8[]
    MessageStart = 2
};

/** All potential results for a network send or receive. */
enum class NetworkResult {
    Success,
    /* Used for when a send or receive was attempted and
     * the peer was found to be disconnected. */
    Disconnected,
    /* Used for when a receive is attempted but there is no data waiting. */
    NoWaitingData
};
/** The NetworkResult and associated data from a network receive. */
struct ReceiveResult {
    NetworkResult result;
    // message will be nullptr if result != Success.
    BinaryBufferPtr message;
};

} /* End namespace AM */

#endif /* End NETWORKDEFS_H */
