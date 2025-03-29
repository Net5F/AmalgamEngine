#pragma once

#include "EngineMessageType.h"
#include "TilePosition.h"
#include "TileOffset.h"
#include "TileLayer.h"
#include "NetworkID.h"
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
    /** The position of the tile to update. */
    TilePosition tilePosition{};

    /** If type == Floor or Object, this is how far the layer should be offset 
        from tilePosition.
        Note: Terrain and Walls don't use this. Terrain is always aligned to 
              the tile, and Walls always match the Terrain height. */
    TileOffset tileOffset{};

    /** The type of tile layer that should be added. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** The numeric ID of the graphic set that the new layer should use.
        If the type is Floor, the existing floor will be overwritten. */
    Uint16 graphicSetID{0};

    /** The graphic value that the new layer should use.
        For all types except Terrain, this is simply an index into 
        graphicSet.graphics. For Terrain, this is a bit-packed value.
        For Terrain, cast this to Terrain::Value. For Walls, cast this to 
        Wall::Type. For Floors and Objects, cast this to Rotation::Direction. */
    Uint8 graphicValue{0};

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
    serializer.object(tileAddLayer.tilePosition);
    serializer.object(tileAddLayer.tileOffset);
    serializer.value1b(tileAddLayer.layerType);
    serializer.value2b(tileAddLayer.graphicSetID);
    serializer.value1b(tileAddLayer.graphicValue);
}

} // End namespace AM
