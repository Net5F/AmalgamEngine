#pragma once

#include "ItemID.h"
#include "ItemInteractionRequest.h"
#include "CombineItems.h"
#include "UseItemOnEntityRequest.h"
#include "ItemRequest.h"
#include "ItemChangeRequest.h"
#include "NetworkDefs.h"
#include "QueuedEvents.h"
#include <queue>

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
class ISimulationExtension;

/**
 * Manages item definitions, handling change requests and requests for data.
 */
class ItemSystem
{
public:
    ItemSystem(Simulation& inSimulation, Network& inNetwork);

    /**
     * Processes the interactions that every item supports (UseOn, Destroy, 
     * and Examine).
     */
    void processItemInteractions();

    /**
     * Processes item definition updates and requests.
     */
    void processItemUpdates();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Sends the given item's examine text to the given client.
     */
    void examineItem(ItemID targetItemID, NetworkID clientID);

    /**
     * Tries to combine the given items in the given player's inventory. If the 
     * combination is invalid (either slot is empty, or neither items supports 
     * the combination), sends an appropriate response.
     */
    void combineItems(Uint8 sourceSlotIndex, Uint8 targetSlotIndex,
                      NetworkID clientID);

    /**
     * Tries to use the item in the given player's inventory slot on the given 
     * entity. If the entity doesn't support the interaction, sends an 
     * appropriate response.
     */
    void useItemOnEntity(Uint8 sourceSlotIndex, entt::entity targetEntity,
                         NetworkID clientID);

    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for receiving requests and sending item and inventory data. */
    Network& network;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if item change requests are valid. */
    ISimulationExtension* extension;

    std::queue<ItemInteractionRequest> destroyInteractionQueue;
    std::queue<ItemInteractionRequest> examineInteractionQueue;

    EventQueue<CombineItems> combineItemsQueue;
    EventQueue<UseItemOnEntityRequest> useItemOnEntityRequestQueue;
    EventQueue<ItemRequest> itemRequestQueue;
    EventQueue<ItemChangeRequest> itemChangeRequestQueue;
};

} // namespace Server
} // namespace AM
