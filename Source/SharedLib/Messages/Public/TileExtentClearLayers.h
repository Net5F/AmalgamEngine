#pragma once

#include "EngineMessageType.h"
#include "TileExtent.h"
#include "TileLayer.h"
#include "NetworkDefs.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that layers be cleared from all tiles in an
 * extent, or by the server to tell a client that an extent of tiles was
 * cleared.
 */
struct TileExtentClearLayers {
public:
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::TileExtentClearLayers};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The tiles to update. */
    TileExtent tileExtent{};

    /** The layer types that should be cleared from the specified tiles. */
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
void serialize(S& serializer, TileExtentClearLayers& tileExtentClearLayers)
{
    serializer.object(tileExtentClearLayers.tileExtent);

    // Bit pack the input array.
    // It's an array of bools, so we can make it pretty small.
    // Note: We expect the outer context (such as EntityUpdate) to
    //       enable bit packing.
    serializer.enableBitPacking([&tileExtentClearLayers](
                                    typename S::BPEnabledType& sbp) {
        sbp.container(tileExtentClearLayers.layerTypesToClear,
                      [](typename S::BPEnabledType& sbp, bool& clearLayerType) {
                          sbp.boolValue(clearLayerType);
                      });
    });

    // Align after bit-packing to make sure the following bytes can be easily
    // processed.
    serializer.adapter().align();
}

} // End namespace AM
