#pragma once

#include "EngineMessageType.h"
#include "TileLayers.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that a layer be removed from a tile, or by the 
 * server to tell a client that a layer was removed.
 */
struct TileRemoveLayer {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{EngineMessageType::TileRemoveLayer};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The X coordinate of the tile to update. */
    int tileX{0};

    /** The Y coordinate of the tile to update. */
    int tileY{0};

    /** The type of tile layer that should be removed. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** The numeric ID of the layer's sprite set.
        For Floors, this will always be 0.
        For Walls, this won't be used (we only need the Wall::Type). */
    Uint16 spriteSetID{0};

    /** The index of the layer's sprite, within the layer's sprite set.
        For Floors, this will always be 0.
        For FloorCoverings and Objects, this should be cast to Rotation.
        For Walls, this should be cast to Wall::Type. */
    Uint8 spriteIndex{0};

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
void serialize(S& serializer, TileRemoveLayer& tileRemoveLayer)
{
    serializer.value4b(tileRemoveLayer.tileX);
    serializer.value4b(tileRemoveLayer.tileY);
    serializer.value1b(tileRemoveLayer.layerType);
    serializer.value2b(tileRemoveLayer.spriteSetID);
    serializer.value1b(tileRemoveLayer.spriteIndex);
}

} // End namespace AM
