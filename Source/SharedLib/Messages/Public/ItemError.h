#pragma once

#include "EngineMessageType.h"
#include "Item.h"
#include <string>
#include <SDL_stdinc.h>

namespace AM
{

/**
 * Used by the server to tell a client that an error occurred during a requested
 * item operation.
 */
struct ItemError {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ItemError};

    enum Type : Uint8 {
        NotSet,
        /** The requested item numeric ID was not found. */
        NumericIDNotFound,
        /** The requested item string ID was not found. */
        StringIDNotFound,
        /** Init request failed because given string ID is already in use. */
        StringIDInUse,
        /** The requester lacks permissions to make the requested change. */
        PermissionFailure,
        /** The given init script failed to execute. */
        InitScriptFailure,
    };

    /** The display name of the relevant item, if it has one. */
    std::string displayName{"Null"};

    /** The string ID of the relevant item, if it has one. */
    std::string stringID{"null"};

    /** The numeric ID of the relevant item, if it has one. */
    ItemID numericID{NULL_ITEM_ID};

    /** The type of error that occurred. */
    Type errorType{};
};

template<typename S>
void serialize(S& serializer, ItemError& itemError)
{
    serializer.text1b(itemError.displayName, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.text1b(itemError.stringID, Item::MAX_DISPLAY_NAME_LENGTH);
    serializer.value2b(itemError.numericID);
    serializer.value1b(itemError.errorType);
}

} // End namespace AM
