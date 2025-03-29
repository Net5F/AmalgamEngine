#pragma once

#include "EngineMessageType.h"
#include "InventoryAddItem.h"
#include "InventoryRemoveItem.h"
#include "InventoryMoveItem.h"
#include "NetworkID.h"
#include "bitsery/ext/std_variant.h"
#include <variant>

namespace AM
{
/**
 * Sent by a client to request that an inventory operation be performed, or by
 * the server to tell a client that an operation was performed.
 *
 * Note: We combine all of these into a single message so that they get
 *       sequenced correctly (since we use per-message queues, relative
 *       ordering may be lost).
 *       If this continues to be a problem, we may want to consider switching
 *       to a messaging architecture that preserves ordering.
 */
struct InventoryOperation {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::InventoryOperation};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    using OperationVariant = std::variant<InventoryAddItem, InventoryRemoveItem,
                                          InventoryMoveItem>;
    OperationVariant operation{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, InventoryOperation& inventoryOperation)
{
    // Note: This calls serialize() for each type.
    serializer.ext(inventoryOperation.operation, bitsery::ext::StdVariant{});
}

} // namespace AM
