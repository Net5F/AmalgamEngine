#pragma once

#include "EngineMessageType.h"
#include "TileLayers.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Used to request that a layer be added to a tile, or to inform a client 
 * that a layer was added.
 */
struct TileAddLayer {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{EngineMessageType::TileAddLayer};

    /** The X coordinate of the tile to update. */
    int tileX{0};

    /** The Y coordinate of the tile to update. */
    int tileY{0};

    /** The type of tile layer that should be added. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** The numeric ID of the sprite set that the new layer should use.
        If the type is Floor, the existing floor will be overwritten. */
    Uint16 spriteSetID{0};

    /** The index within spriteSet.sprites that the new layer should use.
        For Floors, this will always be 0.
        For FloorCoverings and Objects, this should be cast to Rotation.
        For Walls, this should be cast to Wall::Type. */
    Uint8 spriteIndex{0};

    //--------------------------------------------------------------------------
    // Non-replicated data
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
void serialize(S& serializer, TileAddLayer& tileAddLayer)
{
    serializer.value4b(tileAddLayer.tileX);
    serializer.value4b(tileAddLayer.tileY);
    serializer.value1b(tileAddLayer.layerType);
    serializer.value2b(tileAddLayer.spriteSetID);
    serializer.value1b(tileAddLayer.spriteIndex);
}

} // End namespace AM
