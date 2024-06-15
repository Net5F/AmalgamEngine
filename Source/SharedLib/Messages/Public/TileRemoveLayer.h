#pragma once

#include "EngineMessageType.h"
#include "TilePosition.h"
#include "TileLayer.h"
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
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::TileRemoveLayer};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The position of the tile to update. */
    TilePosition tilePosition{};

    /** The type of tile layer that should be removed. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** The numeric ID of the layer's graphic set.
        For Floors, this will always be 0.
        For Walls, this won't be used (we only need the Wall::Type). */
    Uint16 graphicSetID{0};

    /** The layer's graphic value.
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
void serialize(S& serializer, TileRemoveLayer& tileRemoveLayer)
{
    serializer.object(tileRemoveLayer.tilePosition);
    serializer.value1b(tileRemoveLayer.layerType);
    serializer.value2b(tileRemoveLayer.graphicSetID);
    serializer.value1b(tileRemoveLayer.graphicValue);
}

} // End namespace AM
