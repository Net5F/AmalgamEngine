#ifndef NETWORK_H
#define NETWORK_H

#include "messend.hpp"
#include <string>
#include <memory>
#include <vector>
#include <cstddef>

namespace msnd {
    class Message;
    class Peer;
}

namespace AM
{

class Network
{
public:
    Network();

    virtual ~Network();

    void send(std::shared_ptr<msnd::Peer> client, const msnd::Message& messageToSend);

    std::unique_ptr<msnd::Message> receive(std::shared_ptr<msnd::Peer> client);

    /**
     * Accepts any new clients.
     *
     * @return A vector of newly connected clients.
     */
    std::vector<std::shared_ptr<msnd::Peer>> acceptNewClients();

    void checkForDisconnections();

    const std::vector<std::shared_ptr<msnd::Peer>>& getClients();

private:
    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    msnd::Acceptor acceptor;

    std::vector<std::shared_ptr<msnd::Peer>> clients;
};

} // namespace AM

#endif /* NETWORK_H */
