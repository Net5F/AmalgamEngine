#include "World.h"
#include "IDPool.h"
#include <iostream>

AM::World::World()
: entityNames {},
  positions {},
  movements {},
  inputs {},
  sprites {},
  componentFlags {},
  entityIsDirty {}
{
}

AM::EntityID AM::World::AddEntity(const std::string& name)
{
    EntityID id = IDPool::reserveID();
    entityNames[id] = name;
    return id;
}

void AM::World::RemoveEntity(EntityID entityID)
{
    componentFlags[entityID] = 0;
    entityNames[entityID] = "";
}

void AM::World::AttachComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
{
    // If the entity doesn't have the component, add it.
    if ((componentFlags[entityID] & componentFlag) == 0) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        std::cerr << "Tried to add component when entity already has it." << std::endl;
    }
}

void AM::World::RemoveComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
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
