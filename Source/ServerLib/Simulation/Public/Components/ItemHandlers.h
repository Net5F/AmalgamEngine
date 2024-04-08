#pragma once

#include "ItemID.h"
#include "EntityItemHandlerScript.h"
#include <vector>
#include <string_view>

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
    /** Used as a "we should never hit this" cap on the number of item 
        handlers that a component can have assigned.
        Only checked in debug builds. */
    static constexpr std::size_t MAX_HANDLERS{500};

    /**
     * A single item + handler pair.
     */
    struct HandlerPair {
        /** The item that this handler is for. */
        ItemID itemToHandleID{NULL_ITEM_ID};

        /** The handler script to run. */
        EntityItemHandlerScript handlerScript{};
    };

    /** The handlers for each item that this entity supports. */
    std::vector<HandlerPair> handlerPairs{};

    /**
     * Adds the given handler for the given item.
     *
     * Note: Multiple handlers for the same item may exist.
     */
    void add(ItemID itemToHandleID, std::string_view handlerScript)
    {
        handlerPairs.emplace_back(
            itemToHandleID,
            EntityItemHandlerScript{std::string{handlerScript}});
    }
};

template<typename S>
void serialize(S& serializer, ItemHandlers::HandlerPair& handlerPair)
{
    serializer.value2b(handlerPair.itemToHandleID);
    serializer.object(handlerPair.handlerScript);
}

template<typename S>
void serialize(S& serializer, ItemHandlers& itemHandlers)
{
    serializer.container(itemHandlers.handlerPairs, ItemHandlers::MAX_HANDLERS);
}

} // namespace Server
} // namespace AM
