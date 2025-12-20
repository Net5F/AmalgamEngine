#include "World.h"
#include "SimulationContext.h"
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
World::World(const SimulationContext& inSimContext)
: registry{}
, avRegistry{}
, playerEntity{entt::null}
, entityLocator{registry}
, collisionLocator{}
, tileMap{inSimContext.graphicData, collisionLocator}
, castHelper{inSimContext.simulation, inSimContext.network,
             inSimContext.itemData, inSimContext.castableData}
{
    // Initialize our entt groups, before anyone tries to use them.
    EnttGroups::init(registry);
}

} // End namespace Client
} // End namespace AM
