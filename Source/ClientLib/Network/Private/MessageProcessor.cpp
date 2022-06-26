#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "ClientNetworkDefs.h"
#include "ExplicitConfirmation.h"
#include "ConnectionResponse.h"
#include "MovementUpdate.h"
#include "ChunkUpdate.h"
#include "TileUpdate.h"
#include "EntityInit.h"
#include "EntityDelete.h"
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
    // Match the enum values to their event types.
    switch (messageType) {
        case MessageType::ExplicitConfirmation: {
            handleExplicitConfirmation(messageBuffer, messageSize);
            break;
        }
        case MessageType::ConnectionResponse: {
            handleConnectionResponse(messageBuffer, messageSize);
            break;
        }
        case MessageType::MovementUpdate: {
            handleMovementUpdate(messageBuffer, messageSize);
            break;
        }
        case MessageType::ChunkUpdate: {
            dispatchMessageSharedPtr<ChunkUpdate>(messageBuffer, messageSize,
                                                  networkEventDispatcher);
            break;
        }
        case MessageType::TileUpdate: {
            dispatchMessage<TileUpdate>(messageBuffer, messageSize,
                                                  networkEventDispatcher);
            break;
        }
        case MessageType::EntityInit: {
            dispatchMessage<EntityInit>(messageBuffer, messageSize,
                                                  networkEventDispatcher);

            break;
        }
        case MessageType::EntityDelete: {
            dispatchMessage<EntityDelete>(messageBuffer, messageSize,
                                          networkEventDispatcher);
            break;
        }
        default: {
            // If we don't have a handler for this message type, pass it to 
            // the project.
            if (extension != nullptr) {
                extension->processReceivedMessage(messageType, messageBuffer,
                                                  messageSize);
            }
        }
    }
}

void MessageProcessor::setExtension(std::unique_ptr<IMessageProcessorExtension> inExtension)
{
    extension = std::move(inExtension);
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

void MessageProcessor::handleMovementUpdate(Uint8* messageBuffer,
                                            unsigned int messageSize)
{
    // Deserialize the message.
    std::shared_ptr<MovementUpdate> movementUpdate{
        std::make_shared<MovementUpdate>()};
    Deserialize::fromBuffer(messageBuffer, messageSize, *movementUpdate);

    // Pull out the vector of entities.
    const std::vector<MovementState>& entities{movementUpdate->movementStates};

    // Iterate through the entities, checking if there's player or npc data.
    bool playerFound{false};
    bool npcFound{false};
    for (auto entityIt = entities.begin(); entityIt != entities.end();
         ++entityIt) {
        entt::entity entity{entityIt->entity};

        if (entity == playerEntity) {
            // Found the player.
            networkEventDispatcher.push<std::shared_ptr<const MovementUpdate>>(
                movementUpdate);
            playerFound = true;
        }
        else if (!npcFound) {
            // Found a non-player (npc).
            networkEventDispatcher.emplace<NpcUpdate>(NpcUpdateType::Update,
                                                      movementUpdate);
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
            movementUpdate->tickNum);
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
