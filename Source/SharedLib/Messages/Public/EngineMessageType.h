#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of messages that we send across the network.
 *
 * For message descriptions, see their definitions in Shared/Messages/Public.
 */
enum class EngineMessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet,

    // Client -> Server Messages
    Heartbeat,
    ConnectionRequest,
    InputChangeRequest,
    NameChangeRequest,
    AnimationStateChangeRequest,
    ChunkUpdateRequest,
    EntityInitRequest,
    EntityDeleteRequest,
    InitScriptRequest,
    EntityInteractionRequest,
    ItemInteractionRequest,
    ItemRequest,
    ItemChangeRequest,
    UseItemOnEntityRequest,

    // Server -> Client Messages
    ExplicitConfirmation,
    ConnectionResponse,
    SystemMessage,
    EntityInit,
    EntityDelete,
    MovementUpdate,
    ComponentUpdate,
    ChunkUpdate,
    InitScriptResponse,
    InventoryInit,
    Item,

    // Bidirectional Messages
    TileAddLayer,
    TileRemoveLayer,
    TileClearLayers,
    TileExtentClearLayers,
    InventoryAddItem,
    InventoryDeleteItem,
    InventoryMoveItem,
    CombineItems,

    // We reserve values 0 - 124. The project can start at 125.
    PROJECT_START = 125
};

} // End namespace AM
