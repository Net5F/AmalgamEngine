#include "World.h"
#include "Log.h"

namespace AM
{
namespace Server
{
World::World()
: entityNames{}
, positions{}
, movements{}
, inputs{}
, sprites{}
, clients{}
, componentFlags{}
, entityIsDirty{}
, idPool(MAX_ENTITIES)
, device()
, generator(device())
, xDistribution(0, (SCREEN_WIDTH - 128))
, yDistribution(0, (SCREEN_HEIGHT - 128))
{
}

EntityID World::addEntity(std::string_view name)
{
    EntityID id = idPool.reserveID();
    entityNames[id] = name;
    return id;
}

void World::removeEntity(EntityID entityID)
{
    if (entityID > MAX_ENTITIES) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                  entityID);
    }

    componentFlags[entityID] = 0;
    entityNames[entityID] = "";

    idPool.freeID(entityID);
}

void World::attachComponent(EntityID entityID,
                            ComponentFlag::FlagType componentFlag)
{
    if (entityID > MAX_ENTITIES) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                  entityID);
    }

    // If the entity doesn't have the component, add it.
    if ((componentFlags[entityID] & componentFlag) == 0) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        LOG_ERROR("Tried to add component when entity already has it.");
    }
}

void World::removeComponent(EntityID entityID,
                            ComponentFlag::FlagType componentFlag)
{
    if (entityID > MAX_ENTITIES) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                  entityID);
    }

    // If the entity has the component, remove it.
    if ((componentFlags[entityID] & componentFlag) == componentFlag) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        LOG_ERROR("Tried to remove component when entity doesn't have it.");
    }
}

Position World::getSpawnPoint()
{
    return {xDistribution(generator), yDistribution(generator)};
}

} // namespace Server
} // namespace AM
