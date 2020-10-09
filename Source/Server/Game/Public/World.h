#pragma once

#include "GameDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"
#include "ClientComponent.h"
#include "IDPool.h"

#include <array>
#include <map>
#include <string_view>

namespace AM
{
namespace Server
{
class World
{
public:
    World();

    EntityID addEntity(std::string_view name);

    void removeEntity(EntityID entityID);

    /**
     * Registers the entity as possessing this component.
     * The caller is in charge of making sure the state of the component is
     * appropriate.
     */
    void attachComponent(EntityID entityID,
                         ComponentFlag::FlagType componentFlag);

    void removeComponent(EntityID entityID,
                         ComponentFlag::FlagType componentFlag);

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    std::map<EntityID, ClientComponent> clients;
    /**
     * Bit flags for every component, indicating whether the object at a given
     * index has that component.
     */
    std::array<Uint32, MAX_ENTITIES> componentFlags;
    /**
     * Indicates whether we've received new input data for the entity.
     * Since input gets processed before movement, we assume that this also
     * shows which entities need to be broadcast after moving.
     */
    std::array<bool, MAX_ENTITIES> entityIsDirty;

    void setSpawnPoint(const Position& position);
    const Position& getSpawnPoint();

private:
    /** Used for generating entity IDs. */
    IDPool idPool;

    /** The initial spawn point for connecting players. */
    Position spawnPoint;
};

} // namespace Server
} // namespace AM
