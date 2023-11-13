#include "ItemSystem.h"
#include "World.h"
#include "Network.h"
#include "Log.h"

namespace AM
{
namespace Client
{
ItemSystem::ItemSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, itemQueue{network.getEventDispatcher()}
{
}

void ItemSystem::processItemUpdates()
{
    // Process any waiting item definition updates.
    Item item{};
    while (itemQueue.pop(item)) {
        // If the item exists, update it.
        if (world.itemData.itemExists(item.numericID)) {
            world.itemData.updateItem(item);
        }
        else {
            // Item doesn't exist, create it.
            world.itemData.createItem(item);
        }
    }
}

} // namespace Client
} // namespace AM
