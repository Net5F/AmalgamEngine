#pragma once

#include "NetworkDefs.h"
#include <unordered_map>
#include <memory>

/**
 * This file contains client-specific network definitions.
 */
namespace AM
{
namespace Server
{
//--------------------------------------------------------------------------
// Typedefs
//--------------------------------------------------------------------------
/** A map type used to manage clients. */
class Client;
typedef std::unordered_map<NetworkID, std::shared_ptr<Client>> ClientMap;

//--------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------
/**
 * Used after receiving messages from a client to defer processing until later.
 * When we eventually do process the message, we need to optionally update the
 * Client's tick diff info.
 */
struct ClientMessage {
    NetworkID netID{0};
    std::weak_ptr<Client> clientPtr;
    Message message{MessageType::NotSet, nullptr};
};

/**
 * Used to tell the simulation that a client was connected.
 */
struct ClientConnected
{
    /** The ID of the client that connected. */
    NetworkID clientID{0};
};

/**
 * Used to tell the simulation that a client was disconnected.
 */
struct ClientDisconnected
{
    /** The ID of the client that disconnected. */
    NetworkID clientID{0};
};

/**
 * Used to tell the simulation that a client's message arrived late and we
 * had to drop it.
 */
struct ClientMessageDropped
{
    /** The ID of the client that we had to drop a message from. */
    NetworkID clientID{0};
};

} // End namespace Server
} // End namespace AM
