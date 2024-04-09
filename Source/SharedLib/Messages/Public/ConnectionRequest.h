#pragma once

#include "EngineMessageType.h"
#include <string>

namespace AM
{
/**
 * Contains a connection response, sent from the server to the client.
 */
struct ConnectionRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ConnectionRequest};

    /** Used as a "we should never hit this" cap on the size of each name
        string. */
    static constexpr std::size_t MAX_NAME_LENGTH{50};

    // Note: This will eventually change to login credentials and will be sent
    //       to the login server instead of the simulation server.
    /** The name of this player. */
    std::string playerName{""};
};

template<typename S>
void serialize(S& serializer, ConnectionRequest& connectionResponse)
{
    serializer.text1b(connectionResponse.playerName,
                      ConnectionRequest::MAX_NAME_LENGTH);
}

} // End namespace AM
