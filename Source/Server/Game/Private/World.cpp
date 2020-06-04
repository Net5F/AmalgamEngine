#include "World.h"
#include <iostream>

namespace AM
{
namespace Server
{

World::World()
: entityNames {},
  positions {},
  movements {},
  inputs {},
  sprites {},
  componentFlags {},
  entityIsDirty {},
  idPool(),
  spawnPoint {0, 0}
{
}

EntityID World::addEntity(const std::string& name)
{
    EntityID id = idPool.reserveID();
    entityNames[id] = name;
    return id;
}

void World::removeEntity(EntityID entityID)
{
    componentFlags[entityID] = 0;
    entityNames[entityID] = "";

    idPool.freeID(entityID);
}

void World::attachComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
{
    // If the entity doesn't have the component, add it.
    if ((componentFlags[entityID] & componentFlag) == 0) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        std::cerr << "Tried to add component when entity already has it." << std::endl;
    }
}

void World::removeComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
{
    // If the entity has the component, remove it.
    if ((componentFlags[entityID] & componentFlag) == componentFlag) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        std::cerr << "Tried to remove component when entity doesn't have it."
        << std::endl;
    }
}

void World::setSpawnPoint(const Position& newSpawnPoint)
{
    spawnPoint = newSpawnPoint;
}

const Position& World::getSpawnPoint()
{
    return spawnPoint;
}

} // namespace Server
} // namespace AM
