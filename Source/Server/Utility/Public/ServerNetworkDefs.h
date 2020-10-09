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
    // TEMP: Only here until C++20 where emplacing brace lists is allowed.
    ClientMessage(const std::weak_ptr<Client>& inClientPtr, Message inMessage)
    : clientPtr(inClientPtr)
    , message(std::move(inMessage))
    {
    }

    std::weak_ptr<Client> clientPtr;
    Message message = {MessageType::NotSet, nullptr};
};

} // End namespace Server
} // End namespace AM
