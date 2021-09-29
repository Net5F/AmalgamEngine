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

} // End namespace Server
} // End namespace AM
