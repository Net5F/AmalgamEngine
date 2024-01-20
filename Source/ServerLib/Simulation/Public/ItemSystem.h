#pragma once

#include "ItemID.h"
#include "ItemInteractionRequest.h"
#include "CombineItemsRequest.h"
#include "UseItemOnEntityRequest.h"
#include "ItemInitRequest.h"
#include "ItemChangeRequest.h"
#include "ItemDataRequest.h"
#include "NetworkDefs.h"
#include "QueuedEvents.h"

namespace sol
{
class state;
}

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
class ISimulationExtension;
struct EntityItemHandlerScript;

/**
 * Manages item definitions, handling change requests and requests for data.
 */
class ItemSystem
{
public:
    ItemSystem(Simulation& inSimulation, Network& inNetwork,
               sol::state& inEntityItemHandlerLua);

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
     * Adds the given item to updatedItems.
     */
    void itemUpdated(ItemID itemID);

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
     * Creates a new item with the given data. If the ID is already taken,
     * sends an ItemError.
     */
    void handleInitRequest(const ItemInitRequest& itemInitRequest);

    /**
     * Overwrites an existing item with the given data. If no existing item
     * matches the given ID, sends an ItemError.
     */
    void handleChangeRequest(const ItemChangeRequest& itemChangeRequest);

    /**
     * If the requested item exists, sends an update to the requester.
     * If not, sends an ItemError.
     */
    void handleDataRequest(const ItemDataRequest& itemDataRequest);

    /**
     * Runs the given init script on the given item.
     *
     * @return true if the script ran successfully, else false.
     *         If false, an appropriate error message will be sent.
     */
    bool runItemInitScript(NetworkID clientID, const ItemInitScript& initScript,
                           Item& item);

    /**
     * Runs the given item handler script on the given target entity.
     *
     * If the script fails, an appropriate error message will be sent.
     *
     * @param clientID The client that initiated this interaction.
     * @param clientEntity The client's entity.
     * @param targetEntity The entity that the item is being used on.
     */
    void runEntityItemHandlerScript(
        NetworkID clientID, entt::entity clientEntity,
        entt::entity targetEntity,
        const EntityItemHandlerScript& itemHandlerScript);

    /** Used for getting Examine interaction requests. */
    Simulation& simulation;
    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for receiving requests and sending item and inventory data. */
    Network& network;
    /** Used to run entity item handler scripts. */
    sol::state& entityItemHandlerLua;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if item item requests are valid. */
    ISimulationExtension* extension;

    /** Holds a history of items that have been updated.
        Used to know which items need to be sent to clients. */
    std::vector<ItemID> updatedItems;

    EventQueue<ItemInitRequest> itemInitRequestQueue;
    EventQueue<ItemChangeRequest> itemChangeRequestQueue;
    EventQueue<CombineItemsRequest> combineItemsRequestQueue;
    EventQueue<UseItemOnEntityRequest> useItemOnEntityRequestQueue;
    EventQueue<ItemDataRequest> itemDataRequestQueue;
};

} // namespace Server
} // namespace AM
