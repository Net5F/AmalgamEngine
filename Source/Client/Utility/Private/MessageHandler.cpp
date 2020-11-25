#include "MessageHandler.h"
#include "Network.h"
#include "MessageTools.h"
#include "ConnectionResponse.h"
#include "EntityUpdate.h"
#include "MessageDropInfo.h"
#include "Entity.h"
#include "Log.h"

namespace AM
{
namespace Client
{

MessageHandler::MessageHandler(Network& inNetwork)
: network(inNetwork)
{
}

void MessageHandler::handleConnectionResponse(BinaryBuffer& messageRecBuffer,
                                              Uint16 messageSize)
{
    // Deserialize the message.
    std::unique_ptr<ConnectionResponse> connectionResponse
        = std::make_unique<ConnectionResponse>();
    MessageTools::deserialize(messageRecBuffer, messageSize,
                              *connectionResponse);

    // Grab our player ID so we can determine which update messages are for
    // the player.
    network.setPlayerID(connectionResponse->entityID);

    // Queue the message.
    if (!(connectionResponseQueue.enqueue(std::move(connectionResponse)))) {
        LOG_ERROR("Ran out of room in queue and memory allocation failed.");
    }
}

void MessageHandler::handleEntityUpdate(BinaryBuffer& messageRecBuffer,
                                        Uint16 messageSize)
{
    // Deserialize the message.
    std::shared_ptr<EntityUpdate> entityUpdate
        = std::make_shared<EntityUpdate>();
    MessageTools::deserialize(messageRecBuffer, messageSize, *entityUpdate);

    // Pull out the vector of entities.
    const std::vector<Entity>& entities = entityUpdate->entities;

    // Iterate through the entities, checking if there's player or npc data.
    bool playerFound = false;
    bool npcFound = false;
    EntityID playerID = network.getPlayerID();
    for (auto entityIt = entities.begin(); entityIt != entities.end();
         ++entityIt) {
        EntityID entityID = entityIt->id;

        if (entityID == playerID) {
            // Found the player.
            if (!(playerUpdateQueue.enqueue(entityUpdate))) {
                LOG_ERROR("Ran out of room in queue and memory allocation "
                          "failed.");
            }
            playerFound = true;
        }
        else if (!npcFound) {
            // Found a non-player (npc).
            // Queueing the message will let all npc updates within be
            // processed.
            if (!(npcUpdateQueue.enqueue(
                    {NpcUpdateType::Update, entityUpdate}))) {
                LOG_ERROR("Ran out of room in queue and memory allocation "
                          "failed.");
            }
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
        if (!(npcUpdateQueue.enqueue({NpcUpdateType::ImplicitConfirmation,
                                      nullptr, entityUpdate->tickNum}))) {
            LOG_ERROR(
                "Ran out of room in queue and memory allocation failed.");
        }
    }
}

void MessageHandler::handleMessageDropInfo(BinaryBuffer& messageRecBuffer,
                                           Uint16 messageSize)
{
    LOG_INFO("Received MessageDropInfo.");
}

} // End namespace Client
} // End namespace AM
