#pragma once

#include "ItemID.h"
#include "ItemInteractionRequest.h"
#include "CombineItemsRequest.h"
#include "UseItemOnEntityRequest.h"
#include "ItemInitRequest.h"
#include "ItemUpdateRequest.h"
#include "NetworkDefs.h"
#include "QueuedEvents.h"

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
    void examineItem(const Item* item, NetworkID clientID);

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

    /**
     * Builds a new item with the given data. If an existing item has the same 
     * ID, overwrites it. If not, creates a new item.
     */
    void handleInitRequest(const ItemInitRequest& itemInitRequest);

    /**
     * If the requested item exists, sends an update to the requester.
     * If not, sends an ItemError.
     */
    void handleUpdateRequest(const ItemUpdateRequest& itemUpdateRequest);

    /** Used for getting Examine interaction requests. */
    Simulation& simulation;
    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for receiving requests and sending item and inventory data. */
    Network& network;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if item item requests are valid. */
    ISimulationExtension* extension;

    EventQueue<ItemInitRequest> itemInitRequestQueue;
    EventQueue<CombineItemsRequest> combineItemsRequestQueue;
    EventQueue<UseItemOnEntityRequest> useItemOnEntityRequestQueue;
    EventQueue<ItemUpdateRequest> itemUpdateRequestQueue;
};

} // namespace Server
} // namespace AM
