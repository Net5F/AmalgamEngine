#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "ClientNetworkDefs.h"
#include "ExplicitConfirmation.h"
#include "ConnectionResponse.h"
#include "EntityUpdate.h"
#include "ChunkUpdate.h"
#include "TileUpdate.h"
#include "Log.h"

namespace AM
{
namespace Client
{
MessageProcessor::MessageProcessor(EventDispatcher& inNetworkEventDispatcher)
: networkEventDispatcher{inNetworkEventDispatcher}
, playerEntity{entt::null}
{
}

void MessageProcessor::processReceivedMessage(MessageType messageType,
                                              Uint8* messageBuffer,
                                              unsigned int messageSize)
{
    /* Match the enum values to their event types. */
    switch (messageType) {
        case MessageType::ExplicitConfirmation: {
            handleExplicitConfirmation(messageBuffer, messageSize);
            break;
        }
        case MessageType::ConnectionResponse: {
            handleConnectionResponse(messageBuffer, messageSize);
            break;
        }
        case MessageType::EntityUpdate: {
            handleEntityUpdate(messageBuffer, messageSize);
            break;
        }
        case MessageType::ChunkUpdate: {
            pushEventSharedPtr<ChunkUpdate>(messageBuffer, messageSize);
            break;
        }
        case MessageType::TileUpdate: {
            pushEvent<TileUpdate>(messageBuffer, messageSize);
            break;
        }
        default: {
            LOG_ERROR("Received unexpected message type: %u", messageType);
        }
    }
}

template<typename T>
void MessageProcessor::pushEvent(Uint8* messageBuffer, unsigned int messageSize)
{
    // Deserialize the message.
    T message{};
    Deserialize::fromBuffer(messageBuffer, messageSize, message);

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<T>(message);
}

template<typename T>
void MessageProcessor::pushEventSharedPtr(Uint8* messageBuffer,
                                          unsigned int messageSize)
{
    // Deserialize the message.
    std::shared_ptr<T> message{std::make_shared<T>()};
    Deserialize::fromBuffer(messageBuffer, messageSize, *message);

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<std::shared_ptr<const T>>(message);
}

void MessageProcessor::handleExplicitConfirmation(Uint8* messageBuffer,
                                                  unsigned int messageSize)
{
    // Deserialize the message.
    ExplicitConfirmation explicitConfirmation{};
    Deserialize::fromBuffer(messageBuffer, messageSize, explicitConfirmation);

    // Push confirmations into the NPC update system's queue.
    for (unsigned int i = 0; i < explicitConfirmation.confirmedTickCount; ++i) {
        networkEventDispatcher.emplace<NpcUpdate>(
            NpcUpdateType::ExplicitConfirmation);
    }
}

void MessageProcessor::handleEntityUpdate(Uint8* messageBuffer,
                                          unsigned int messageSize)
{
    // Deserialize the message.
    std::shared_ptr<EntityUpdate> entityUpdate
        = std::make_shared<EntityUpdate>();
    Deserialize::fromBuffer(messageBuffer, messageSize, *entityUpdate);

    // Pull out the vector of entities.
    const std::vector<EntityState>& entities = entityUpdate->entityStates;

    // Iterate through the entities, checking if there's player or npc data.
    bool playerFound = false;
    bool npcFound = false;
    for (auto entityIt = entities.begin(); entityIt != entities.end();
         ++entityIt) {
        entt::entity entity = entityIt->entity;

        if (entity == playerEntity) {
            // Found the player.
            networkEventDispatcher.push<std::shared_ptr<const EntityUpdate>>(
                entityUpdate);
            playerFound = true;
        }
        else if (!npcFound) {
            // Found a non-player (npc).
            networkEventDispatcher.emplace<NpcUpdate>(NpcUpdateType::Update,
                                                      entityUpdate);
            npcFound = true;
        }

        // If we found the player and an npc, we can stop looking.
        if (playerFound && npcFound) {
            break;
        }
    }

    // If we didn't find an NPC and queue an update message, push an
    // implicit confirmation to show that we've confirmed up to this tick.
    if (!npcFound) {
        networkEventDispatcher.emplace<NpcUpdate>(
            NpcUpdateType::ImplicitConfirmation, nullptr,
            entityUpdate->tickNum);
    }
}

void MessageProcessor::handleConnectionResponse(Uint8* messageBuffer,
                                                unsigned int messageSize)
{
    // Deserialize the message.
    ConnectionResponse connectionResponse{};
    Deserialize::fromBuffer(messageBuffer, messageSize, connectionResponse);

    // Save our player entity so we can determine which update messages are for
    // the player.
    playerEntity = connectionResponse.entity;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<ConnectionResponse>(connectionResponse);
}

} // End namespace Client
} // End namespace AM
