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
    ChunkUpdateRequest,
    DynamicObjectInitRequest,
    InitScriptRequest,
    InteractionRequest,

    // Server -> Client Messages
    ExplicitConfirmation,
    ConnectionResponse,
    MovementUpdate,
    ChunkUpdate,
    ClientEntityInit,
    DynamicObjectInit,
    InitScriptResponse,

    // Bidirectional Messages
    TileAddLayer,
    TileRemoveLayer,
    TileClearLayers,
    TileExtentClearLayers,
    EntityDelete,
    SpriteChange,

    // We reserve values 0 - 124. The project can start at 125.
    PROJECT_START = 125
};

} // End namespace AM
