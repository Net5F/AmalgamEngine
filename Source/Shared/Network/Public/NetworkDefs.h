#pragma once

#include <SDL2/SDL_stdinc.h>
#include <memory>
#include <vector>

/**
 * This file contains shared network type definitions that should be
 * consistent between the server and client.
 */
namespace AM
{
//--------------------------------------------------------------------------
// Typedefs
//--------------------------------------------------------------------------
/** Represents a single network client. Will be reused if the client
    disconnects. */
typedef Uint32 NetworkID;

/** Dynamically allocated, portable buffers for messages. */
typedef std::vector<Uint8> BinaryBuffer;
typedef std::unique_ptr<BinaryBuffer> BinaryBufferPtr;
typedef std::shared_ptr<BinaryBuffer> BinaryBufferSharedPtr;

//--------------------------------------------------------------------------
// Headers
//--------------------------------------------------------------------------
/**
 * Used for indexing into the parts of a server header.
 */
struct ServerHeaderIndex {
    enum Index : Uint8 {
        /** Sint8, the adjustment that the server wants the client to make. */
        TickAdjustment = 0,
        /** Uint8, the iteration of tick offset adjustment that we're on. */
        AdjustmentIteration = 1,
        /** Uint8, the number of messages in this batch. */
        MessageCount = 2,
        /** Uint8, the number of ticks that have passed with no update. */
        ConfirmedTickCount = 3,
        /** The start of the first message header if one is present. */
        MessageHeaderStart = 4
    };
};
/** The size of a server header in bytes. */
static constexpr unsigned int SERVER_HEADER_SIZE
    = ServerHeaderIndex::MessageHeaderStart;

/** Used for indexing into the parts of a client header. */
struct ClientHeaderIndex {
    enum Index : Uint8 {
        /** Uint8, the iteration of tick offset adjustment that we're on. */
        AdjustmentIteration = 0,
        /** The start of the first message header if one is present. */
        MessageHeaderStart = 1
    };
};
/** The size of a client header in bytes. */
static constexpr unsigned int CLIENT_HEADER_SIZE
    = ClientHeaderIndex::MessageHeaderStart;

/** Used for indexing into the size or payload of a received message. */
struct MessageHeaderIndex {
    enum Index : Uint8 {
        /** Uint8, identifies the type of message. */
        MessageType = 0,
        /** Uint16, the size of the message in bytes. */
        Size = 1,
        /** The start of the message payload bytes. */
        MessageStart = 3
    };
};
static constexpr unsigned int MESSAGE_HEADER_SIZE
    = MessageHeaderIndex::MessageStart;

//--------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------
/** All potential results for a network send or receive. */
enum class NetworkResult {
    /** Used for initialization, indicates the value hasn't been set. */
    NotSet,
    /** Used for when a message was successfully sent or received. */
    Success,
    /* Used for when a send or receive was attempted and
       the peer was found to be disconnected. */
    Disconnected,
    /* Used for when a receive is attempted but there is no data waiting. */
    NoWaitingData
};

/**
 * The type of message to expect. For descriptions, see their definitions in
 * Shared/Messages/Public.
 */
enum class MessageType : Uint8 {
    /** Used for initialization, indicates the value hasn't been set. */
    NotSet = 0,
    ConnectionResponse = 1,
    EntityUpdate = 2,
    ClientInputs = 3,
    Heartbeat = 4,
};

/** Represents the result of trying to receive a message. */
struct MessageResult {
    NetworkResult networkResult = NetworkResult::NotSet;
    /** messageType will be Invalid if networkResult != Success. */
    MessageType messageType = MessageType::NotSet;
    /** If networkResult == Success, contains the size of the received message.
     */
    Uint16 messageSize = 0;
};

/**
 * Represents a serialized message. Useful if we want to defer deserialization
 * till later, maybe because we're in a time-critical context.
 */
struct Message {
    MessageType messageType = MessageType::NotSet;
    BinaryBufferPtr messageBuffer = nullptr;
};

} // End namespace AM
