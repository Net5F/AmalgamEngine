#include "World.h"
#include "Simulation.h"
#include "Network.h"
#include "GraphicData.h"
#include "ItemData.h"
#include "CastableData.h"
#include "EnttGroups.h"

namespace AM
{
namespace Client
{
World::World(Simulation& inSimulation, Network& inNetwork,
             const GraphicData& inGraphicData, const ItemData& inItemData,
             const CastableData& inCastableData)
: registry{}
, playerEntity{entt::null}
, entityLocator{registry}
, collisionLocator{}
, tileMap{inGraphicData, collisionLocator}
, castHelper{inSimulation, inNetwork, inItemData, inCastableData}
{
    // Initialize our entt groups, before anyone tries to use them.
    EnttGroups::init(registry);
}

} // End namespace Client
} // End namespace AM
