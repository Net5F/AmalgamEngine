#pragma once

#include "MessageType.h"
#include <string>

namespace AM
{
/**
 * Contains a connection response, sent from the server to the client.
 */
struct ConnectionRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::ConnectionRequest;

    // Note: This will eventually change to login credentials and will be sent 
    //       to the login server instead of the simulation server.
    /** The name of this player. */
    std::string playerName{""};
};

template<typename S>
void serialize(S& serializer, ConnectionRequest& connectionResponse)
{
    serializer.value(connectionResponse.playerName);
}

} // End namespace AM
