#pragma once

#include "EngineMessageType.h"
#include "TilePosition.h"
#include "TileLayer.h"
#include "ConnectionHandle.h"
#include <SDL3/SDL_stdinc.h>

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
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
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
