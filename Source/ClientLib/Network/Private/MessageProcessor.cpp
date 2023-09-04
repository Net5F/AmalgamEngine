#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "IMessageProcessorExtension.h"
#include "ClientNetworkDefs.h"
#include "ExplicitConfirmation.h"
#include "ConnectionResponse.h"
#include "UserErrorString.h"
#include "MovementUpdate.h"
#include "ChunkUpdate.h"
#include "ClientEntityInit.h"
#include "DynamicObjectInit.h"
#include "InitScriptResponse.h"
#include "EntityDelete.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "SpriteChange.h"
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

void MessageProcessor::processReceivedMessage(Uint8 messageType,
                                              Uint8* messageBuffer,
                                              std::size_t messageSize)
{
    // Match the enum values to their event types.
    EngineMessageType engineMessageType{
        static_cast<EngineMessageType>(messageType)};
    switch (engineMessageType) {
        case EngineMessageType::ExplicitConfirmation: {
            handleExplicitConfirmation(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::ConnectionResponse: {
            handleConnectionResponse(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::UserErrorString: {
            dispatchMessage<UserErrorString>(messageBuffer, messageSize,
                                             networkEventDispatcher);
            break;
        }
        case EngineMessageType::MovementUpdate: {
            handleMovementUpdate(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::ChunkUpdate: {
            dispatchMessageSharedPtr<ChunkUpdate>(messageBuffer, messageSize,
                                                  networkEventDispatcher);
            break;
        }
        case EngineMessageType::ClientEntityInit: {
            dispatchMessage<ClientEntityInit>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::DynamicObjectInit: {
            dispatchMessage<DynamicObjectInit>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::InitScriptResponse: {
            dispatchMessage<InitScriptResponse>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityDelete: {
            dispatchMessage<EntityDelete>(messageBuffer, messageSize,
                                          networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileAddLayer: {
            dispatchMessage<TileAddLayer>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileRemoveLayer: {
            dispatchMessage<TileRemoveLayer>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileClearLayers: {
            dispatchMessage<TileClearLayers>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::TileExtentClearLayers: {
            dispatchMessage<TileExtentClearLayers>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::SpriteChange: {
            dispatchMessage<SpriteChange>(messageBuffer, messageSize,
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
            break;
        }
    }
}

void MessageProcessor::setExtension(
    std::unique_ptr<IMessageProcessorExtension> inExtension)
{
    extension = std::move(inExtension);
}

void MessageProcessor::handleExplicitConfirmation(Uint8* messageBuffer,
                                                  std::size_t messageSize)
{
    // Deserialize the message.
    ExplicitConfirmation explicitConfirmation{};
    Deserialize::fromBuffer(messageBuffer, messageSize, explicitConfirmation);

    // Push confirmations into the NPC update system's queue.
    for (std::size_t i = 0; i < explicitConfirmation.confirmedTickCount; ++i) {
        networkEventDispatcher.emplace<NpcUpdate>(
            NpcUpdateType::ExplicitConfirmation);
    }
}

void MessageProcessor::handleMovementUpdate(Uint8* messageBuffer,
                                            std::size_t messageSize)
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
                                                std::size_t messageSize)
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
