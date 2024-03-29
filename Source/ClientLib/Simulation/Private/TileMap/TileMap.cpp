#include "TileMap.h"
#include "GraphicData.h"
#include "Paths.h"
#include "Position.h"
#include "Transforms.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "TileMapSnapshot.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Timer.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Client
{
TileMap::TileMap(GraphicData& inGraphicData)
: TileMapBase{inGraphicData, false}
, sizeChangedSig{}
, sizeChanged{sizeChangedSig}
{
    if (Config::RUN_OFFLINE) {
        LOG_INFO("Offline mode. Constructing default tile map.");

        // Set our map size.
        setMapSize(1, 1);

        // If we have any floor graphic sets, fill the map with the first one.
        const std::vector<FloorGraphicSet>& floorGraphicSets{
            graphicData.getAllFloorGraphicSets()};
        if (floorGraphicSets.size() > 0) {
            const FloorGraphicSet& floorGraphicSet{floorGraphicSets[0]};
            for (int x = tileExtent.x; x <= tileExtent.xMax(); ++x) {
                for (int y = tileExtent.y; y <= tileExtent.yMax(); ++y) {
                    setFloor(x, y, floorGraphicSet);
                }
            }
        }
    }
}

void TileMap::setMapSize(std::size_t inMapXLengthChunks,
                         std::size_t inMapYLengthChunks)
{
    // Set our map size.
    // Note: We set x/y to 0 since our map origin is always (0, 0). Change
    //       this if we ever support negative origins.
    chunkExtent.x = 0;
    chunkExtent.y = 0;
    chunkExtent.xLength = static_cast<int>(inMapXLengthChunks);
    chunkExtent.yLength = static_cast<int>(inMapYLengthChunks);
    tileExtent.x = 0;
    tileExtent.y = 0;
    tileExtent.xLength = (chunkExtent.xLength * SharedConfig::CHUNK_WIDTH);
    tileExtent.yLength = (chunkExtent.yLength * SharedConfig::CHUNK_WIDTH);

    // Resize the tiles vector to fit the map.
    tiles.resize(tileExtent.xLength * tileExtent.yLength);

    // Signal that the size changed.
    sizeChangedSig.publish(tileExtent);
}

} // End namespace Client
} // End namespace AM
