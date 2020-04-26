#include <World.h>
#include <iostream>

namespace AM
{
namespace Client
{

World::World()
: entityNames {},
  positions {},
  movements {},
  inputs {},
  sprites {},
  componentFlags {},
  playerID(0)
{
}

void World::AddEntity(const std::string& name, EntityID ID)
{
    entityNames[ID] = name;
}

void World::RemoveEntity(EntityID entityID)
{
    componentFlags[entityID] = 0;
    entityNames[entityID] = "";
}

bool World::entityExists(EntityID entityID) const
{
    return (componentFlags[entityID] == 0) ? false : true;
}

void World::AttachComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
{
    // If the entity doesn't have the component, add it.
    if ((componentFlags[entityID] & componentFlag) == 0) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        std::cerr << "Tried to add component when entity already has it." << std::endl;
    }
}

void World::RemoveComponent(EntityID entityID, ComponentFlag::FlagType componentFlag)
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

void World::registerPlayerID(EntityID inPlayerID)
{
    playerID = inPlayerID;
}

EntityID World::getPlayerID()
{
    return playerID;
}

} // namespace Client
} // namespace AM
