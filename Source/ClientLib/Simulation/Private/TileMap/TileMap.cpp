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
            for (int x{tileExtent.x}; x <= tileExtent.xMax(); ++x) {
                for (int y{tileExtent.y}; y <= tileExtent.yMax(); ++y) {
                    setFloor({x, y, 0}, floorGraphicSet);
                }
            }
        }
    }
}

void TileMap::setMapSize(Uint16 inMapXLengthChunks, Uint16 inMapYLengthChunks,
                         Uint16 inMapZLengthChunks)
{
    // Set our map size.
    chunkExtent = ChunkExtent::fromMapLengths(
        inMapXLengthChunks, inMapYLengthChunks, inMapZLengthChunks);
    tileExtent = TileExtent{chunkExtent};

    // Signal that the size changed.
    sizeChangedSig.publish(tileExtent);
}

} // End namespace Client
} // End namespace AM
