#include "TileUpdateSystem.h"

namespace AM
{
namespace Client
{

TileUpdateSystem::TileUpdateSystem(Simulation& inSim, World& inWorld
                 , EventDispatcher& inUiEventDispatcher
                 , EventDispatcher& inNetworkEventDispatcher
                 , Network& inNetwork, SpriteData& inSpriteData)
: sim{inSim}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, tileUpdateRequestQueue(inUiEventDispatcher)
, tileUpdateQueue(inNetworkEventDispatcher)
{
}

void TileUpdateSystem::updateTiles()
{
//    world.tileMap.setSpriteLayer(tileIndex.x, tileIndex.y, 0,
//                                     *(terrainSprites[terrainSpriteIndex]));
}

} // End namespace Client
} // End namespace AM
