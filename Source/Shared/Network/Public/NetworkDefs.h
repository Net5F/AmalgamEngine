#pragma once

#include "BinaryBuffer.h"
#include "MessageType.h"
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
        /** The start of the first message header if one is present. */
        MessageHeaderStart = 3
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
/** All potential results for a network-layer send or receive. */
enum class NetworkResult {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,
    /** A message was successfully sent or received. */
    Success,
    /* A send or receive was attempted and the peer was found to be
       disconnected. */
    Disconnected,
    /* A receive was attempted but there was no data waiting. */
    NoWaitingData,
    /* We timed out while waiting for a send or receive. */
    TimedOut,
};

/**
 * Info resulting from an attempt to receive a message.
 */
struct ReceiveResult {
    /** The result of the receive attempt. */
    NetworkResult networkResult{NetworkResult::NotSet};

    /** The type of the received message. Will be Invalid if networkResult
        != Success. */
    MessageType messageType{MessageType::NotSet};

    /** If networkResult == Success, contains the size of the received message.
     */
    Uint16 messageSize{0};
};

} // End namespace AM
