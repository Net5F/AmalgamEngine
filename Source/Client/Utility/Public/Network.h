#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <memory>

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

    bool connect();

    void send(const msnd::Message& messageToSend);

    std::unique_ptr<msnd::Message> receive();

private:
    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::unique_ptr<msnd::Peer> server;
};

} // namespace AM

#endif /* NETWORK_H */
