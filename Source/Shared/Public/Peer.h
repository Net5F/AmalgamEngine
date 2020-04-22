#ifndef PEER_H_
#define PEER_H_

#include "SharedDefs.h"
#include <SDL2/SDL_net.h>
#include <memory>
#include <array>

namespace AM
{

/**
 * Represents a network peer for communication.
 */
class Peer
{
public:
    static constexpr unsigned int MAX_MESSAGE_SIZE = 4000;

    /**
     * Allows the client to connect to the server.
     */
    static std::unique_ptr<Peer> initiate(std::string serverIP, unsigned int serverPort);

    Peer(TCPsocket inSocket);

    ~Peer();

    bool isConnected();

    bool sendMessage(BinaryBufferSharedPtr message);

    BinaryBufferPtr receiveMessage();

    BinaryBufferPtr receiveMessageWait();

private:
    SDLNet_SocketSet set;
    TCPsocket socket;

    bool peerIsConnected;

    std::array<Uint8, MAX_MESSAGE_SIZE> messageBuffer;
};

} /* End namespace AM */

#endif /* End PEER_H_ */
