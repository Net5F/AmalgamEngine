#pragma once

#include "ChunkPosition.h"
#include "ChunkSnapshot.h"
#include "bitsery/ext/std_map.h"
#include <unordered_map>

namespace AM
{
/**
 * Holds tile map data in a persistable form.
 *
 * Used in saving/loading the tile map.
 *
 * This struct is fairly similar to the normal representation of the tile map,
 * see ChunkSnapshot and TileSnapshot for more obvious differences.
 */
struct TileMapSnapshot {
    /** Used as a "we should never hit this" cap on the number of chunks in a
        map. Currently set to 64*64*10. */
    static constexpr std::size_t MAX_CHUNKS{40960};

    /** The version of the map format. Kept as just a 16-bit int for now, we
        can see later if we care to make it more complicated. */
    Uint16 version{0};

    // Note: The map's origin is currently always assumed to be (0, 0). Add
    //       x/y fields here if we ever support negative origins.

    /** The length, in chunks, of the map's X axis. */
    Uint16 xLengthChunks{0};

    /** The length, in chunks, of the map's Y axis. */
    Uint16 yLengthChunks{0};

    /** The length, in chunks, of the map's Z axis. */
    Uint16 zLengthChunks{0};

    /** The chunks that make up this map. */
    std::unordered_map<ChunkPosition, ChunkSnapshot> chunks;
};

template<typename S>
void serialize(S& serializer, TileMapSnapshot& tileMapSnapshot)
{
    serializer.value2b(tileMapSnapshot.version);
    serializer.value2b(tileMapSnapshot.xLengthChunks);
    serializer.value2b(tileMapSnapshot.yLengthChunks);
    serializer.value2b(tileMapSnapshot.zLengthChunks);
    serializer.ext(tileMapSnapshot.chunks,
                   bitsery::ext::StdMap{TileMapSnapshot::MAX_CHUNKS},
                   [](S& serializer, ChunkPosition& chunkPosition,
                      ChunkSnapshot& chunkSnapshot) {
                       serializer.object(chunkPosition);
                       serializer.object(chunkSnapshot);
                   });
}

} // End namespace AM
