#include "MessageProcessor.h"
#include "QueuedEvents.h"
#include "Deserialize.h"
#include "DispatchMessage.h"
#include "ExplicitConfirmation.h"
#include "ConnectionResponse.h"
#include "SystemMessage.h"
#include "EntityInit.h"
#include "EntityDelete.h"
#include "EntityInitScriptResponse.h"
#include "MovementUpdate.h"
#include "ComponentUpdate.h"
#include "ChunkUpdate.h"
#include "InventoryInit.h"
#include "CastCooldownInit.h"
#include "ItemError.h"
#include "ItemUpdate.h"
#include "ItemInitScriptResponse.h"
#include "CombineItems.h"
#include "DialogueResponse.h"
#include "CastFailed.h"
#include "CastStarted.h"
#include "TileAddLayer.h"
#include "TileRemoveLayer.h"
#include "TileClearLayers.h"
#include "TileExtentClearLayers.h"
#include "InventoryOperation.h"
#include "PlayerMovementUpdate.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Client
{
MessageProcessor::MessageProcessor(EventDispatcher& inNetworkEventDispatcher)
: networkEventDispatcher{inNetworkEventDispatcher}
, playerEntity{entt::null}
, lastReceivedTick{0}
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
        case EngineMessageType::SystemMessage: {
            dispatchMessage<SystemMessage>(messageBuffer, messageSize,
                                           networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInit: {
            dispatchMessage<EntityInit>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityDelete: {
            dispatchMessage<EntityDelete>(messageBuffer, messageSize,
                                          networkEventDispatcher);
            break;
        }
        case EngineMessageType::EntityInitScriptResponse: {
            dispatchMessage<EntityInitScriptResponse>(
                messageBuffer, messageSize, networkEventDispatcher);
            break;
        }
        case EngineMessageType::MovementUpdate: {
            handleMovementUpdate(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::ComponentUpdate: {
            handleComponentUpdate(messageBuffer, messageSize);
            break;
        }
        case EngineMessageType::ChunkUpdate: {
            dispatchMessageSharedPtr<ChunkUpdate>(messageBuffer, messageSize,
                                                  networkEventDispatcher);
            break;
        }
        case EngineMessageType::InventoryInit: {
            dispatchMessage<InventoryInit>(messageBuffer, messageSize,
                                           networkEventDispatcher);
            break;
        }
        case EngineMessageType::CastCooldownInit: {
            dispatchMessage<CastCooldownInit>(messageBuffer, messageSize,
                                              networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemError: {
            dispatchMessage<ItemError>(messageBuffer, messageSize,
                                       networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemUpdate: {
            dispatchMessage<ItemUpdate>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::ItemInitScriptResponse: {
            dispatchMessage<ItemInitScriptResponse>(messageBuffer, messageSize,
                                                    networkEventDispatcher);
            break;
        }
        case EngineMessageType::CombineItems: {
            dispatchMessage<CombineItems>(messageBuffer, messageSize,
                                          networkEventDispatcher);
            break;
        }
        case EngineMessageType::DialogueResponse: {
            dispatchMessage<DialogueResponse>(messageBuffer, messageSize,
                                              networkEventDispatcher);
            break;
        }
        case EngineMessageType::CastFailed: {
            dispatchMessage<CastFailed>(messageBuffer, messageSize,
                                        networkEventDispatcher);
            break;
        }
        case EngineMessageType::CastStarted: {
            dispatchMessage<CastStarted>(messageBuffer, messageSize,
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
        case EngineMessageType::InventoryOperation: {
            dispatchMessage<InventoryOperation>(messageBuffer, messageSize,
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

Uint32 MessageProcessor::getLastReceivedTick()
{
    return lastReceivedTick;
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

    // Move lastReceivedTick forward.
    lastReceivedTick += explicitConfirmation.confirmedTickCount;
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

    // Initialize lastReceivedTick.
    lastReceivedTick = connectionResponse.tickNum;

    // Push the message into any subscribed queues.
    networkEventDispatcher.push<ConnectionResponse>(connectionResponse);
}

void MessageProcessor::handleMovementUpdate(Uint8* messageBuffer,
                                            std::size_t messageSize)
{
    // Deserialize the message.
    std::shared_ptr<MovementUpdate> movementUpdate{
        std::make_shared<MovementUpdate>()};
    Deserialize::fromBuffer(messageBuffer, messageSize, *movementUpdate);

    // If the message's tick is newer than our saved tick, update it.
    if (movementUpdate->tickNum > lastReceivedTick) {
        lastReceivedTick = movementUpdate->tickNum;
    }
    AM_ASSERT((movementUpdate->tickNum >= lastReceivedTick),
              "Received ticks out of order. last: %u, new: %u",
              lastReceivedTick.load(), movementUpdate->tickNum);

    // If the player entity is present, process it and erase it from the
    // message.
    std::vector<MovementState>& movementStates{movementUpdate->movementStates};
    for (auto it = movementStates.begin(); it != movementStates.end(); ++it) {
        MovementState& movementState{*it};
        if (movementState.entity == playerEntity) {
            PlayerMovementUpdate playerMovementUpdate{
                {movementState.entity, movementState.input,
                 movementState.position, movementState.movement},
                movementUpdate->tickNum};
            networkEventDispatcher.push(playerMovementUpdate);

            movementStates.erase(it);
            break;
        }
    }

    // If there are NPC entities remaining in the message, push it.
    if (movementStates.size() > 0) {
        networkEventDispatcher.push<std::shared_ptr<const MovementUpdate>>(
            movementUpdate);
    }
}

void MessageProcessor::handleComponentUpdate(Uint8* messageBuffer,
                                             std::size_t messageSize)
{
    // Deserialize the message.
    ComponentUpdate componentUpdate{};
    Deserialize::fromBuffer(messageBuffer, messageSize, componentUpdate);

    // If the message's tick is newer than our saved tick, update it.
    if (componentUpdate.tickNum > lastReceivedTick) {
        lastReceivedTick = componentUpdate.tickNum;
    }
    AM_ASSERT((componentUpdate.tickNum >= lastReceivedTick),
              "Received ticks out of order. last: %u, new: %u",
              lastReceivedTick.load(), componentUpdate.tickNum);

    networkEventDispatcher.push(componentUpdate);
}

} // End namespace Client
} // End namespace AM
