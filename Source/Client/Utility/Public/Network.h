#ifndef NETWORK_H
#define NETWORK_H

#include "SharedDefs.h"
#include <string>
#include <memory>

namespace AM
{

class Peer;

namespace Client
{

class Network
{
public:
    Network();

    virtual ~Network();

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

} // namespace Client
} // namespace AM

#endif /* NETWORK_H */
