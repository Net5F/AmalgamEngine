#pragma once

#include "EngineMessageType.h"
#include "TileLayers.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that a layer be added to a tile, or by the server
 * to tell a client that a layer was added.
 */
struct TileAddLayer {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::TileAddLayer};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The X coordinate of the tile to update. */
    int tileX{0};

    /** The Y coordinate of the tile to update. */
    int tileY{0};

    /** The type of tile layer that should be added. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** The numeric ID of the graphic set that the new layer should use.
        If the type is Floor, the existing floor will be overwritten. */
    Uint16 graphicSetID{0};

    /** The index within graphicSet.graphics that the new layer should use.
        For Floors, this will always be 0.
        For FloorCoverings and Objects, this should be cast to Rotation.
        For Walls, this should be cast to Wall::Type. */
    Uint8 graphicIndex{0};

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
void serialize(S& serializer, TileAddLayer& tileAddLayer)
{
    serializer.value4b(tileAddLayer.tileX);
    serializer.value4b(tileAddLayer.tileY);
    serializer.value1b(tileAddLayer.layerType);
    serializer.value2b(tileAddLayer.graphicSetID);
    serializer.value1b(tileAddLayer.graphicIndex);
}

} // End namespace AM
