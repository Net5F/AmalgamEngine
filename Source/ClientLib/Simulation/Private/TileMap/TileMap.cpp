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
        setMapSize(1, 1, 1);

        // If we have any floor graphic sets, fill the map with the first one.
        const std::vector<FloorGraphicSet>& floorGraphicSets{
            graphicData.getAllFloorGraphicSets()};
        if (floorGraphicSets.size() > 0) {
            const FloorGraphicSet& floorGraphicSet{floorGraphicSets[0]};
            for (int z{tileExtent.z}; z <= tileExtent.zMax(); ++z) {
                for (int y{tileExtent.y}; y <= tileExtent.yMax(); ++y) {
                    for (int x{tileExtent.x}; x <= tileExtent.xMax(); ++x) {
                        setFloor({x, y, z}, floorGraphicSet);
                    }
                }
            }
        }
    }
}

void TileMap::setMapSize(Uint16 inMapXLengthChunks, Uint16 inMapYLengthChunks,
                         Uint16 inMapZLengthChunks)
{
    // Set our map size.
    chunkExtent.xLength = inMapXLengthChunks;
    chunkExtent.yLength = inMapYLengthChunks;
    chunkExtent.zLength = inMapZLengthChunks;
    chunkExtent.x = -(chunkExtent.xLength / 2);
    chunkExtent.y = -(chunkExtent.yLength / 2);
    chunkExtent.z = 0;

    tileExtent.xLength = (chunkExtent.xLength * SharedConfig::CHUNK_WIDTH);
    tileExtent.yLength = (chunkExtent.yLength * SharedConfig::CHUNK_WIDTH);
    tileExtent.zLength = chunkExtent.zLength;
    tileExtent.x = (chunkExtent.x * SharedConfig::CHUNK_WIDTH);
    tileExtent.y = (chunkExtent.y * SharedConfig::CHUNK_WIDTH);
    tileExtent.z = 0;

    // Resize the chunks vector to fit the map.
    chunks.resize(chunkExtent.getCount());

    // Signal that the size changed.
    sizeChangedSig.publish(tileExtent);
}

} // End namespace Client
} // End namespace AM
