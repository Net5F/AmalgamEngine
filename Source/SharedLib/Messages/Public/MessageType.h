#pragma once

#include <SDL_stdinc.h>

namespace AM
{
/**
 * The types of messages that we send across the network.
 *
 * For descriptions, see their definitions in Shared/Messages/Public.
 */
enum class MessageType : Uint8 {
    /** Indicates the value hasn't been set. Used for initialization. */
    NotSet = 0,

    // Client -> Server Messages
    Heartbeat = 1,
    ConnectionRequest = 2,
    InputChangeRequest = 3,
    ChunkUpdateRequest = 4,

    // Server -> Client Messages
    ExplicitConfirmation = 30,
    ConnectionResponse = 31,
    MovementUpdate = 32,
    ChunkUpdate = 33,
    EntityInit = 34,
    EntityDelete = 35,

    // Bidirectional Messages
    TileAddLayer = 60,
    TileRemoveLayer = 61,
    TileClearLayers = 62,
    TileExtentClearLayers = 63,
};

} // End namespace AM
