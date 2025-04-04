#pragma once

#include "EngineMessageType.h"
#include "TilePosition.h"
#include "TileLayer.h"
#include "NetworkID.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * Sent by a client to request that layers be cleared from a tile, or by the
 * server to tell a client that layers were cleared.
 */
struct TileClearLayers {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::TileClearLayers};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The position of the tile to update. */
    TilePosition tilePosition{};

    /** The layer types that should be cleared from the specified tile. */
    std::array<bool, TileLayer::Type::Count> layerTypesToClear{};

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
void serialize(S& serializer, TileClearLayers& tileClearLayers)
{
    serializer.object(tileClearLayers.tilePosition);

    // Bit pack the input array.
    // It's an array of bools, so we can make it pretty small.
    serializer.enableBitPacking([&tileClearLayers](
                                    typename S::BPEnabledType& sbp) {
        sbp.container(tileClearLayers.layerTypesToClear,
                      [](typename S::BPEnabledType& sbp, bool& clearLayerType) {
                          sbp.boolValue(clearLayerType);
                      });
    });
}

} // End namespace AM
