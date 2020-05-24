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
    SDLNet_Quit();
    exitRequested = true;
    receiveThreadObj.join();
}

void Network::send(std::shared_ptr<Peer> client,
                             BinaryBufferSharedPtr message)
{
    sendQueue.push_back({client, message});
}

void Network::sendToAll(BinaryBufferSharedPtr message)
{
    sendQueue.push_back({nullptr, message});
}

void Network::sendWaitingMessages(float deltaSeconds)
{
    accumulatedTime += deltaSeconds;

    if (accumulatedTime >= NETWORK_TICK_INTERVAL_S) {
        sendConnectionResponsesInternal();
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

void Network::eraseDisconnectedClients()
{
    for (auto it = clients.begin(); it != clients.end();) {
       if (!(it->second->isConnected())) {
           it = clients.erase(it);
       }
       else {
           ++it;
       }
    }
}

int Network::processClients(Network* network)
{
    const std::shared_ptr<SDLNet_SocketSet> clientSet = network->getClientSet();
    const std::unordered_map<EntityID, std::shared_ptr<Peer>>& clients =
        network->getClients();

    std::atomic<bool> const* exitRequested = network->getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Check if there are any new clients to connect.
        network->acceptNewClients();

        // Check if we've detected any disconnected clients.
        network->eraseDisconnectedClients();

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
const std::unordered_map<EntityID, std::shared_ptr<Peer>>& clients)
{
    // Check if there are any messages to receive.
    int numReady = SDLNet_CheckSockets(*clientSet, 100);
    if (numReady == -1) {
        DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
        // Most of the time this is a system error, where perror might help.
        perror("SDLNet_CheckSockets");
    }
    else if (numReady > 0) {
        // Receive all messages from all clients.
        for (const auto& pair : clients) {
            BinaryBufferSharedPtr message = pair.second->receiveMessage(false);
            while (message != nullptr) {
                // Queue the message (blocks if the queue is locked).
                network->queueInputMessage(message);

                message = pair.second->receiveMessage(false);
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

void Network::sendConnectionResponsesInternal()
{
    /* Send all waiting ConnectionResponse messages. */
    unsigned int responseCount = connectionResponseQueue.size();
    if (responseCount > 0) {
        Uint32 latestTickTimestamp = *currentTickPtr;
        DebugInfo("Sending latest tick: %u", latestTickTimestamp);

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

            send(clients.at(data.id),
                NetworkHelpers::constructMessage(builder.GetSize(),
                    builder.GetBufferPointer()));
            DebugInfo("Sent new client response with tick = %u", latestTickTimestamp);

            connectionResponseQueue.pop();
        }
    }
}

void Network::sendWaitingMessagesInternal()
{
    /* Send all waiting messages from the standard send queue. */
    while (!sendQueue.empty()) {
        MessageInfo& messageInfo = sendQueue.front();
        if (messageInfo.client != nullptr) {
            // Send to single client.
            if (!messageInfo.client->sendMessage(messageInfo.message)) {
                DebugError("Send failed. Returning, will be attempted again next tick.");
                return;
            }
            else {
                sendQueue.pop_front();
            }
        }
        else {
            // Send to all connected clients.
            for (const auto& pair : clients) {
                if (pair.second->isConnected()) {
                    if (!pair.second->sendMessage(messageInfo.message)) {
                        DebugInfo("Send failed. Client probably disconnected.");
                        return;
                    }
                }
            }

            // Message has been sent to any connected clients, get rid of it.
            sendQueue.pop_front();
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

const std::unordered_map<EntityID, std::shared_ptr<Peer>>& Network::getClients()
{
    return clients;
}

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Server
} // namespace AM
