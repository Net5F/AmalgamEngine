#pragma once

#include "NetworkDefs.h"
#include <unordered_map>

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
    std::weak_ptr<Client> clientPtr;
    Message message;
};

} // End namespace Server
} // End namespace AM
