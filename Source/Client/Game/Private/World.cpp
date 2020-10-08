#include <World.h>
#include "Debug.h"

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
, playerID(0)
, playerIsDirty(false)
, oldPositions{}
{
    // Init the history with default snapshots.
    for (unsigned int i = 0; i < INPUT_HISTORY_LENGTH; ++i) {
        playerInputHistory.push(InputStateArr{});
    }
}

void World::addEntity(std::string_view name, EntityID entityID)
{
    if (entityID > MAX_ENTITIES) {
        DebugError("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                   entityID);
    }

    entityNames[entityID] = name;
}

void World::removeEntity(EntityID entityID)
{
    if (entityID > MAX_ENTITIES) {
        DebugError("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                   entityID);
    }

    componentFlags[entityID] = 0;
    entityNames[entityID] = "";
}

bool World::entityExists(EntityID entityID) const
{
    if (entityID > MAX_ENTITIES) {
        DebugError("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                   entityID);
    }

    return (componentFlags[entityID] == 0) ? false : true;
}

void World::attachComponent(EntityID entityID,
                            ComponentFlag::FlagType componentFlag)
{
    if (entityID > MAX_ENTITIES) {
        DebugError("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                   entityID);
    }

    // If the entity doesn't have the component, add it.
    if ((componentFlags[entityID] & componentFlag) == 0) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        DebugError("Tried to add component when entity already has it.");
    }
}

void World::removeComponent(EntityID entityID,
                            ComponentFlag::FlagType componentFlag)
{
    if (entityID > MAX_ENTITIES) {
        DebugError("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                   entityID);
    }

    // If the entity has the component, remove it.
    if ((componentFlags[entityID] & componentFlag) == componentFlag) {
        componentFlags[entityID] |= componentFlag;
    }
    else {
        DebugError("Tried to remove component when entity doesn't have it.");
    }
}

void World::registerPlayerID(EntityID inPlayerID)
{
    if (inPlayerID > MAX_ENTITIES) {
        DebugError("Invalid entity ID. Max: %u, given: %u", MAX_ENTITIES,
                   inPlayerID);
    }

    playerID = inPlayerID;
}

} // namespace Client
} // namespace AM
