#pragma once

#include "ItemID.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <optional>
#include <string>

namespace AM
{
namespace Server
{
class World;
class Network;

/**
 * Static functions for manipulating inventories.
 * 
 * When inventories are manipulated, the clients that own them need to be 
 * updated. Normally we would observe inventory changes and auto-replicate the 
 * whole component, but doing so with a large inventory would be inefficient.
 * Instead, we use these functions to both update the inventory and send the 
 * necessary client updates.
 */
class InventoryHelpers {
public:
    /**
     * Adds the given item to the first available slot in the given entity's 
     * inventory.
     *
     * @param requesterID The client that requested an item be added, if there 
     *                    was one. If non-null, error messages will be sent on 
     *                    failure.
     * @return true if successful, else false.
     */
    static bool addItem(ItemID itemID, Uint8 count, entt::entity entityToAddTo,
                        World& world, Network& network,
                        std::optional<NetworkID> requesterID = std::nullopt);

    /**
     * Overload for string IDs.
     */
    static bool addItem(const std::string& itemID, Uint8 count, entt::entity entityToAddTo,
                        World& world, Network& network,
                        std::optional<NetworkID> requesterID = std::nullopt);

    /**
     * Removes the item in the given slot in the given entity's inventory.
     *
     * @return true if successful, else false.
     */
    static bool removeItem(Uint8 slotIndex, Uint8 count,
                           entt::entity entityToRemoveFrom, World& world,
                           Network& network);

    /**
     * Overload for string IDs.
     */
    static bool removeItem(const std::string& itemID, Uint8 count,
                           entt::entity entityToRemoveFrom, World& world,
                           Network& network);
};

} // namespace Server
} // namespace AM
