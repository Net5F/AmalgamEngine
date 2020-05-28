#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include "NetworkHelpers.h"
#include <algorithm>
#include "Debug.h"

namespace AM
{
namespace Server
{

Uint32* Network::currentTickPtr = nullptr;

void Network::registerCurrentTickPtr(Uint32* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

Network::Network()
: acceptor(nullptr)
, newClientQueue(MAX_QUEUED_NEW_CLIENTS)
, clientSet(std::make_shared<SDLNet_SocketSet>(SDLNet_AllocSocketSet(MAX_CLIENTS)))
, accumulatedTime(0.0f)
, receiveThreadObj()
, exitRequested(false)
, builder(BUILDER_BUFFER_SIZE)
{
    SDLNet_Init();

    // We delay creating the acceptor because startup has to be called first.
    acceptor = std::make_unique<Acceptor>(SERVER_PORT, clientSet);

    // Start the receive thread.
    receiveThreadObj = std::thread(Network::processClients, this);
}

Network::~Network()
{
    exitRequested = true;
    receiveThreadObj.join();
    SDLNet_Quit();
}

void Network::send(EntityID id, BinaryBufferSharedPtr message)
{
    // Queue the message to be sent with the next batch.
    clients.at(id).queueMessage(message);
}

void Network::sendToAll(BinaryBufferSharedPtr message)
{
    // Queue the message to be sent with the next batch.
    for (auto& pair : clients) {
        pair.second.queueMessage(message);
    }
}

void Network::sendWaitingMessages(float deltaSeconds)
{
    accumulatedTime += deltaSeconds;

    if (accumulatedTime >= NETWORK_TICK_INTERVAL_S) {
        // Queue connection responses before starting to send this batch.
        queueConnectionResponses();

        // Send all messages for this batch.
        sendWaitingMessagesInternal();

        accumulatedTime -= NETWORK_TICK_INTERVAL_S;
        if (accumulatedTime >= NETWORK_TICK_INTERVAL_S) {
            // If we've accumulated enough time to send more, something
            // happened to delay us.
            // We still only want to send what's in the queue, but it's worth giving
            // debug output that we detected this.
            DebugInfo(
                "Detected a delayed network send. accumulatedTime: %f. Setting to 0.",
                accumulatedTime);
            accumulatedTime = 0;
        }
    }
}

std::queue<BinaryBufferSharedPtr>& Network::startReceiveInputMessages(Uint32 tickNum)
{
    return inputMessageSorter.startReceive(tickNum);
}

void Network::endReceiveInputMessages()
{
    inputMessageSorter.endReceive();
}

void Network::acceptNewClients()
{
    std::shared_ptr<Peer> newClient = acceptor->accept();
    while (newClient != nullptr) {
        DebugInfo("New client connected.");

        if (!(newClientQueue.enqueue(newClient))) {
            DebugError(
                "Ran out of room in new client queue and memory allocation failed.");
        }

        // Creates the new client, which adds itself to the socket set.
        newClient = acceptor->accept();
    }
}

void Network::addClient(EntityID entityID, std::shared_ptr<AM::Peer> client)
{
    clients.emplace(entityID, client);
}

void Network::sendConnectionResponse(EntityID id, float spawnX, float spawnY)
{
    connectionResponseQueue.push({id, spawnX, spawnY});
}

int Network::processClients(Network* network)
{
    const std::shared_ptr<SDLNet_SocketSet> clientSet = network->getClientSet();
    std::unordered_map<EntityID, Client>& clients = network->getClients();

    std::atomic<bool> const* exitRequested = network->getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Check if there are any new clients to connect.
        network->acceptNewClients();

        // Check if there's any clients to receive from.
        if (clients.size() == 0) {
            // Delay so we don't waste CPU spinning.
            SDL_Delay(1);
        }
        else {
            receiveClientMessages(network, clientSet, clients);
        }
    }

    return 0;
}

void Network::receiveClientMessages(
Network* network,
const std::shared_ptr<SDLNet_SocketSet> clientSet,
std::unordered_map<EntityID, Client>& clients)
{
    // Check if there are any messages to receive.
    int numReady = SDLNet_CheckSockets(*clientSet, 100);
    if (numReady == -1) {
        DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
        // Most of the time this is a system error, where perror might help.
        perror("SDLNet_CheckSockets");
    }
    else if (numReady > 0) {
        /* Iterate through all clients. */
        for (auto it = clients.begin(); it != clients.end();) {
            Client& client = it->second;

            /* Receive all messages from the client. */
            ReceiveResult receiveResult = client.receiveMessage();
            while (receiveResult.result == NetworkResult::Success) {
                // Queue the message (blocks if the queue is locked).
                network->queueInputMessage(receiveResult.message);

                receiveResult = client.receiveMessage();
            }

            if (receiveResult.result == NetworkResult::Disconnected) {
                DebugInfo("Client disconnected. Removing from map.");
                it = clients.erase(it);
            }
            else {
                // Manually increment the iterator because of the paths that erase elements.
                ++it;
            }
        }
    }
}

void Network::queueInputMessage(BinaryBufferSharedPtr messageBuffer)
{
    const fb::Message* message = fb::GetMessage(messageBuffer->data());
    if (message->content_type() != fb::MessageContent::EntityUpdate) {
        DebugError("Expected EntityUpdate but got something else.");
    }
    auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());

    // Push the message into the queue for its tick.
    DebugInfo("Received message with tick: %u", message->tickTimestamp());
    inputMessageSorter.push(message->tickTimestamp(), messageBuffer);
}

void Network::queueConnectionResponses()
{
    /* Send all waiting ConnectionResponse messages. */
    unsigned int responseCount = connectionResponseQueue.size();
    if (responseCount > 0) {
        Uint32 latestTickTimestamp = *currentTickPtr;

        for (unsigned int i = 0; i < responseCount; ++i) {
            ConnectionResponseData& data = connectionResponseQueue.front();

            // Prep the builder for a new message.
            builder.Clear();

            // Send them their ID and spawn point.
            auto response = fb::CreateConnectionResponse(builder, data.id, data.spawnX,
                data.spawnY);
            auto encodedMessage = fb::CreateMessage(builder, latestTickTimestamp,
                fb::MessageContent::ConnectionResponse, response.Union());
            builder.Finish(encodedMessage);

            // Queue the message so it gets included in the next batch.
            send(data.id,
                NetworkHelpers::constructMessage(builder.GetSize(),
                    builder.GetBufferPointer()));
            DebugInfo("Sent new client response with tick = %u", latestTickTimestamp);

            connectionResponseQueue.pop();
        }
    }
}

void Network::sendWaitingMessagesInternal()
{
    /* Send the waiting messages in every client's queue. */
    for (auto it = clients.begin(); it != clients.end();) {
        Client& client = it->second;

        Uint8 messageCount = client.getWaitingMessageCount();
        if (messageCount > 0) {
            // Build the batch header.
            Uint8 header[SERVER_HEADER_SIZE] = {0, messageCount};

            // Send the batch header.
            NetworkResult result = client.sendHeader(
                std::make_shared<std::vector<Uint8>>(header,
                    header + SERVER_HEADER_SIZE));
            if (result == NetworkResult::Disconnected) {
                DebugInfo(
                    "Send failed, client likely disconnected. Removing client from map.");
                it = clients.erase(it);
                continue; // Continue to avoid confusing iterator state.
            }

            // Send all waiting messages.
            result = client.sendWaitingMessages();
            if (result == NetworkResult::Disconnected) {
                DebugInfo(
                    "Send failed, client likely disconnected. Removing client from map.");
                it = clients.erase(it);
                continue; // Continue to avoid confusing iterator state.
            }

            // Manually increment the iterator because of the paths that erase elements.
            ++it;
        }
    }
}

std::shared_ptr<Peer> Network::getNewClient()
{
    std::shared_ptr<Peer> client = nullptr;
    if (newClientQueue.try_dequeue(client)) {
        return client;
    }
    else {
        return nullptr;
    }
}

const std::shared_ptr<SDLNet_SocketSet> Network::getClientSet()
{
    return clientSet;
}

std::unordered_map<EntityID, Client>& Network::getClients()
{
    return clients;
}

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Server
} // namespace AM
