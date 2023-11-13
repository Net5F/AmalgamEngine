#pragma once

#include "ItemID.h"
#include <functional>
#include <vector>

namespace AM
{
namespace Server
{
/**
 * An entity's handling logic for when an item is used on it.
 *
 * There's no default handler. You must add a handler for a particular item 
 * in order for anything to happen when that item is used on this entity.
 */
struct ItemHandlers {
    /**
     * A single item + handler pair.
     */
    struct HandlerPair {
        /** The item that this handler is for. */
        ItemID itemToHandleID{NULL_ITEM_ID};

        /** The handling logic. */
        std::function<void()> handler{};
    };

    /** The handlers for each item that this entity supports. */
    std::vector<HandlerPair> handlerPairs{};

    /**
     * Adds the given handler for the given item.
     *
     * Note: Multiple handlers for the same item may exist.
     */
    void add(ItemID itemToHandleID, std::function<void()> handler) {
        handlerPairs.emplace_back(itemToHandleID, handler);
    }
};

} // namespace Server
} // namespace AM
