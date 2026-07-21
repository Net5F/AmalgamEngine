#include "ClientManager.h"
#include "asio/io_context.hpp"
#include "SocketSet.h"
#include "Config.h"
#include "MessageProcessor.h"
#include "Log.h"

namespace AM
{
namespace AccountServer
{

ClientManager::ClientManager(asio::io_context& inNetworkIoContext)
: networkIDPool{IDPool::ReservationStrategy::MarchForward, 8}
, networkIoContext{inNetworkIoContext}
, acceptor{inNetworkIoContext,
           asio::ip::tcp::endpoint{asio::ip::tcp::v4(), Config::SERVER_PORT}}
, databasePool{1}
, messageProcessor{inNetworkIoContext, databasePool,
                   std::bind_front(&ClientManager::sendMessage, this)}
{
    networkIDPool.reserveID();

    acceptNewClient();
}

// TODO: What do we need to do?
//         Registration
//         Password recovery
//         Login
//       What data is needed for each client?
//         Done in schema
//       What operations are involved?
//         New account registration request
//           -> Username + password
//           <- Recovery key
//         Login request
//           -> Username + password
//           <- Session token
//         Password reset request
//         New password request

void ClientManager::acceptNewClient()
{
    acceptor.async_accept(
        [this](const asio::error_code& error, asio::ip::tcp::socket socket) {
            if (!error) {
                addClient(std::move(socket));
            }
            else if (error == asio::error::operation_aborted) {
                // Acceptor was cancelled or closed.
                return;
            }
            else {
                LOG_INFO("Accept failed: %s", error.message().c_str());
            }

            acceptNewClient();
        });
}

void ClientManager::addClient(asio::ip::tcp::socket socket)
{
    // If we're at max capacity, reject the connection.
    if (clientMap.size() >= Config::MAX_CLIENTS) {
        LOG_INFO("Rejected connection attempt: Already at maximum "
                 "connected clients.");
        return;
    }

    // Try to create the client and add it to the map.
    NetworkID newID{static_cast<NetworkID>(networkIDPool.reserveID())};
    auto [clientIt, wasSuccessful] = clientMap.try_emplace(
        newID,
        std::make_shared<Client>(
            newID, std::move(socket),
            std::bind_front(&MessageProcessor::processReceivedMessage,
                            &messageProcessor),
            std::bind_front(&ClientManager::eraseDisconnectedClient, this)));
    if (!wasSuccessful) {
        LOG_ERROR("Ran out of room in client map or key already existed.");
        networkIDPool.freeID(newID);
        return;
    }

    // Start the client's receive loop.
    clientIt->second->startReceiveLoop();

    LOG_INFO("New client connected. Assigning netID: %u", newID);
}

void ClientManager::eraseDisconnectedClient(NetworkID netID,
                                            asio::error_code error)
{
    auto clientIt{clientMap.find(netID)};
    if (clientIt == clientMap.end()) {
        return;
    }

    clientMap.erase(clientIt);
    networkIDPool.freeID(netID);

    LOG_INFO("Erased disconnected client with netID: %u.", netID);
}

void ClientManager::sendMessage(NetworkID netID, BinaryBufferSharedPtr message)
{
    auto clientIt{clientMap.find(netID)};
    if (clientIt == clientMap.end()) {
        // Client disconnected while processing the request.
        return;
    }

    clientIt->second->send(std::move(message));
}

} // End namespace AccountServer
} // End namespace AM
