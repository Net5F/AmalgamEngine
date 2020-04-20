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

class World
{
public:
    World();

    EntityID AddEntity(const std::string& name);

    void RemoveEntity(EntityID entityID);

    /**
     * Registers the entity as possessing this component.
     * The caller is in charge of making sure the state of the component is appropriate.
     */
    void AttachComponent(EntityID entityID, ComponentFlag::FlagType componentFlag);

    void RemoveComponent(EntityID entityID, ComponentFlag::FlagType componentFlag);

    /** Entity data lists. */
    std::array<std::string, MAX_ENTITIES> entityNames;
    std::array<PositionComponent, MAX_ENTITIES> positions;
    std::array<MovementComponent, MAX_ENTITIES> movements;
    std::array<InputComponent, MAX_ENTITIES> inputs;
    std::array<SpriteComponent, MAX_ENTITIES> sprites;
    // Bit flags for every component, indicating whether the object at a given index has that component.
    std::array<uint32_t, MAX_ENTITIES> componentFlags;
};

} // namespace AM

#endif /* WORLD_H */
