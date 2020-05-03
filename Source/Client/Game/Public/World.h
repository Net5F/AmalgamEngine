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

    EntityID getPlayerID();

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    // Bit flags for every component, indicating whether the object at a given index has that component.
    std::array<uint32_t, MAX_ENTITIES> componentFlags;

private:
    EntityID playerID;
};

} // namespace Client
} // namespace AM

#endif /* WORLD_H */
