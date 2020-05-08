#ifndef WORLD_H
#define WORLD_H

#include "SharedDefs.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"
#include "SpriteComponent.h"

#include <array>
#include <string>

namespace AM
{
namespace Client
{

class World
{
public:
    World();

    void addEntity(const std::string& name, EntityID ID);

    void removeEntity(EntityID entityID);

    bool entityExists(EntityID entityID) const;

    /**
     * Registers the entity as possessing this component.
     * The caller is in charge of making sure the state of the component is appropriate.
     */
    void attachComponent(EntityID entityID, ComponentFlag::FlagType componentFlag);

    void removeComponent(EntityID entityID, ComponentFlag::FlagType componentFlag);

    /**
     * Registers an entity as being the player. Various systems will only apply to this entity.
     */
    void registerPlayerID(EntityID inPlayerID);

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    // Bit flags for every component, indicating whether the object at a given index has that component.
    std::array<uint32_t, MAX_ENTITIES> componentFlags;

    /** Player-specific. */
    EntityID playerID;

    // We don't need to track NPC dirty states because we don't give them input.
    bool playerIsDirty;

    /** Additional data */
    // Position from the previous tick that the renderer can use to lerp.
    std::array<PositionComponent, MAX_ENTITIES> oldPositions;
};

} // namespace Client
} // namespace AM

#endif /* WORLD_H */
