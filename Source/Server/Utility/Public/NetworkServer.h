#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "SharedDefs.h"
#include <string>
#include <memory>
#include <vector>
#include <cstddef>

namespace AM
{

class Acceptor;
class Peer;

class NetworkServer
{
public:
    NetworkServer();

    virtual ~NetworkServer();

    /**
     * Sends bytes over the network.
     */
    bool send(std::shared_ptr<Peer> client, BinaryBufferSharedPtr message);

    /**
     * Checks for a waiting message for the given client. If one is waiting, packs it into a
     * dynamic buffer and returns it.
     * @return The waiting message if one was present, else nullptr.
     */
    BinaryBufferPtr receive(std::shared_ptr<Peer> client);

    /**
     * Accepts any new clients.
     *
     * @return A vector of newly connected clients.
     */
    std::vector<std::shared_ptr<AM::Peer>> acceptNewClients();

    /**
     * Iterates through the clients and erases any that are disconnected.
     */
    void checkForDisconnections();

    const std::vector<std::shared_ptr<AM::Peer>>& getClients();

private:
    // The predicate used for checking if clients are disconnected.
    static bool IsDisconnected(const std::shared_ptr<Peer>& peer);

    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::unique_ptr<AM::Acceptor> acceptor;

    std::vector<std::shared_ptr<Peer>> clients;
};

} // namespace AM

#endif /* NETWORKSERVER_H */
