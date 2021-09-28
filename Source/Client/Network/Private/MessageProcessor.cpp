#include "MessageProcessor.h"
#include "Network.h"
#include "Deserialize.h"
#include "ExplicitConfirmation.h"
#include "ConnectionResponse.h"
#include "EntityUpdate.h"
#include "UpdateChunks.h"
#include "Log.h"

namespace AM
{
namespace Client
{
MessageProcessor::MessageProcessor(EventDispatcher& inDispatcher)
: dispatcher{inDispatcher}
, playerEntity{entt::null}
{
}

void MessageProcessor::processReceivedMessage(MessageType messageType,
                                              BinaryBuffer& messageBuffer,
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
        case MessageType::UpdateChunks: {
            pushEventSharedPtr<UpdateChunks>(messageBuffer, messageSize);
            break;
        }
        default: {
            LOG_ERROR("Received unexpected message type: %u", messageType);
        }
    }
}

template<typename T>
void MessageProcessor::pushEvent(BinaryBuffer& messageBuffer,
                                 unsigned int messageSize)
{
    // Deserialize the message.
    T message{};
    Deserialize::fromBuffer(messageBuffer, messageSize, message);

    // Push the message into any subscribed queues.
    dispatcher.push<T>(message);
}

template<typename T>
void MessageProcessor::pushEventSharedPtr(BinaryBuffer& messageBuffer,
                                          unsigned int messageSize)
{
    // Deserialize the message.
    std::shared_ptr<T> message{std::make_shared<T>()};
    Deserialize::fromBuffer(messageBuffer, messageSize, *message);

    // Push the message into any subscribed queues.
    dispatcher.push<std::shared_ptr<const T>>(message);
}

void MessageProcessor::handleExplicitConfirmation(BinaryBuffer& messageBuffer,
                                                  Uint16 messageSize)
{
    // Deserialize the message.
    ExplicitConfirmation explicitConfirmation{};
    Deserialize::fromBuffer(messageBuffer, messageSize, explicitConfirmation);

    // Push confirmations into the NPC update system's queue.
    for (unsigned int i = 0; i < explicitConfirmation.confirmedTickCount; ++i) {
        dispatcher.emplace<NpcUpdateMessage>(
            NpcUpdateType::ExplicitConfirmation);
    }
}

void MessageProcessor::handleEntityUpdate(BinaryBuffer& messageBuffer,
                                          Uint16 messageSize)
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
            dispatcher.push<std::shared_ptr<const EntityUpdate>>(entityUpdate);
            playerFound = true;
        }
        else if (!npcFound) {
            // Found a non-player (npc).
            dispatcher.emplace<NpcUpdateMessage>(NpcUpdateType::Update,
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
        dispatcher.emplace<NpcUpdateMessage>(
            NpcUpdateType::ImplicitConfirmation, nullptr,
            entityUpdate->tickNum);
    }
}

void MessageProcessor::handleConnectionResponse(BinaryBuffer& messageBuffer,
                                                Uint16 messageSize)
{
    // Deserialize the message.
    ConnectionResponse connectionResponse{};
    Deserialize::fromBuffer(messageBuffer, messageSize, connectionResponse);

    // Save our player entity so we can determine which update messages are for
    // the player.
    playerEntity = connectionResponse.entity;

    // Push the message into any subscribed queues.
    dispatcher.push<ConnectionResponse>(connectionResponse);
}

} // End namespace Client
} // End namespace AM
