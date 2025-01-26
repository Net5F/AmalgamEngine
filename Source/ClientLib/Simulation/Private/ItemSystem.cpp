#include "ItemSystem.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ItemDataRequest.h"
#include "ItemCache.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "StringTools.h"
#include "Paths.h"
#include "Log.h"
#include <filesystem>

namespace AM
{
namespace Client
{
ItemSystem::ItemSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, itemUpdateQueue{network.getEventDispatcher()}
, combineItemsQueue{network.getEventDispatcher()}
{
    // Load all items from ItemCache.bin into ItemData.
    loadItemCache();

    // When an item is created or updated, save it to the cache.
    world.itemData.itemCreated.connect<&ItemSystem::saveItemCache>(this);
    world.itemData.itemUpdated.connect<&ItemSystem::saveItemCache>(this);
}

void ItemSystem::processItemUpdates()
{
    ItemData& itemData{world.itemData};

    // Process any waiting item definition updates.
    ItemUpdate itemUpdate{};
    while (itemUpdateQueue.pop(itemUpdate)) {
        // Load the given item definition into ItemData, overwriting any 
        // existing item if present.
        Item item{.displayName = itemUpdate.displayName,
                  .numericID = itemUpdate.numericID,
                  .iconID = itemUpdate.iconID,
                  .maxStackSize = itemUpdate.maxStackSize,
                  .supportedInteractions = itemUpdate.supportedInteractions};
        StringTools::deriveStringID(itemUpdate.displayName, item.stringID);
        itemData.loadItem(item, itemUpdate.version);
    }

    // Process any waiting item combinations.
    CombineItems combineItems{};
    while (combineItemsQueue.pop(combineItems)) {
        world.registry.patch<Inventory>(
            world.playerEntity, [&](Inventory& inventory) {
                inventory.combineItems(combineItems.sourceSlotIndex,
                                       combineItems.targetSlotIndex,
                                       combineItems.resultItemID,
                                       combineItems.resultItemMaxStackSize);
            });

        // If we don't have the latest definition for the new item, request
        // it.
        ItemID resultItemID{combineItems.resultItemID};
        if (!(world.itemData.getItem(resultItemID))
            || (world.itemData.getItemVersion(resultItemID)
                < combineItems.resultItemVersion)) {
            network.serializeAndSend(ItemDataRequest{resultItemID});
        }
    }
}

void ItemSystem::loadItemCache()
{
    // If the cache doesn't exist, return early.
    std::string cachePath{Paths::BASE_PATH + "ItemCache.bin"};
    if (!std::filesystem::exists(cachePath)) {
        return;
    }

    // Deserialize the item cache.
    ItemCache itemCache{};
    bool loadSuccessful{Deserialize::fromFile(cachePath, itemCache)};
    if (!loadSuccessful) {
        LOG_FATAL("Failed to deserialize item cache at path: %s",
                  cachePath.c_str());
    }

    // Push all cached items into ItemData.
    for (const ItemCache::ItemEntry& itemEntry : itemCache.items) {
        world.itemData.loadItem(itemEntry.item, itemEntry.version);
    }
}

void ItemSystem::saveItemCache()
{
    // Gather all items from ItemData.
    ItemCache itemCache{};
    const auto& items{world.itemData.getAllItems()};
    itemCache.items.resize(items.size());

    int index{0};
    for (auto& [itemID, item] : items) {
        itemCache.items[index] = {item, world.itemData.getItemVersion(itemID)};
        index++;
    }

    // Serialize the item cache and write it into a file.
    bool saveSuccessful{
        Serialize::toFile((Paths::BASE_PATH + "ItemCache.bin"), itemCache)};
    if (!saveSuccessful) {
        LOG_FATAL("Failed to serialize and save the item cache.");
    }
}

} // namespace Client
} // namespace AM
