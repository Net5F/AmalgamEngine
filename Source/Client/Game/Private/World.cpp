#include "World.h"
#include "Log.h"

namespace AM
{
namespace Client
{
World::World()
: entityNames{}
, positions{}
, movements{}
, inputs{}
, sprites{}
, componentFlags{}
, oldPositions{}
{
    // Init the history with default snapshots.
    for (unsigned int i = 0; i < PlayerData::INPUT_HISTORY_LENGTH; ++i) {
        playerData.inputHistory.push(InputStateArr{});
    }
}

void World::addEntity(std::string_view name, EntityID entityID)
{
    if (entityID > MAX_ENTITY_ID) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITY_ID,
                  entityID);
    }

    entityNames[entityID] = name;
}

void World::removeEntity(EntityID entityID)
{
    if (entityID > MAX_ENTITY_ID) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITY_ID,
                  entityID);
    }

    componentFlags[entityID] = 0;
    entityNames[entityID] = "";
}

bool World::entityExists(EntityID entityID) const
{
    if (entityID > MAX_ENTITY_ID) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITY_ID,
                  entityID);
    }

    return (componentFlags[entityID] == 0) ? false : true;
}

void World::attachComponent(EntityID entityID,
                            ComponentFlag::FlagType componentFlag)
{
    if (entityID > MAX_ENTITY_ID) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITY_ID,
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
    if (entityID > MAX_ENTITY_ID) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITY_ID,
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

void World::registerPlayerID(EntityID inPlayerID)
{
    if (inPlayerID > MAX_ENTITY_ID) {
        LOG_ERROR("Invalid entity ID. Max: %u, given: %u", MAX_ENTITY_ID,
                  inPlayerID);
    }

    playerData.ID = inPlayerID;
}

} // namespace Client
} // namespace AM
