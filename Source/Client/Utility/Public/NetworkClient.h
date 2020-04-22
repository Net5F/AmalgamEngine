#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "SharedDefs.h"
#include <string>
#include <memory>

namespace AM
{

class Peer;

class NetworkClient
{
public:
    NetworkClient();

    virtual ~NetworkClient();

    bool connect();

    /**
     * Sends bytes over the network.
     */
    bool send(BinaryBufferSharedPtr message);

    BinaryBufferPtr receive();

private:
    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::unique_ptr<AM::Peer> server;
};

} // namespace AM

#endif /* NETWORKCLIENT_H */
