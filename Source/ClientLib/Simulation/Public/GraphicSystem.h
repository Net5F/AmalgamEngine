#pragma once

#include "EntityGraphicType.h"
#include "Rotation.h"
#include "entt/fwd.hpp"

namespace AM
{
struct EntityGraphicSet;

namespace Client
{

class World;
class GraphicData;
class ISimulationExtension;
struct ClientGraphicState;

/**
 * Handles updating each entity's graphic state to reflect their current sim 
 * state.
 *
 * Does not render anything (that's handled by Renderer).
 */
class GraphicSystem
{
public:
    GraphicSystem(World& inWorld, GraphicData& inGraphicData);

    /**
     * Updates each entity's sprite to reflect their current state.
     */
    void updateAnimations();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Returns true if the given entity has an Input component and an input 
     * is pressed.
     */
    bool isMoving(entt::entity entity);

    /**
     * Returns the graphic type that an entity with the given state should be 
     * set to, if that entity is also moving.
     */
    EntityGraphicType
        getUpdatedRunGraphicType(const EntityGraphicSet& graphicSet,
                                 const Rotation& rotation,
                                 const ClientGraphicState& clientGraphicState);

    /**
     * Returns the graphic type that an entity with the given state should be 
     * set to, if that entity is also not moving.
     */
    EntityGraphicType
        getUpdatedIdleGraphicType(const EntityGraphicSet& graphicSet,
                                  const Rotation& rotation,
                                  const ClientGraphicState& clientGraphicState);

    /**
     * Converts a graphic type to its associated direction.
     * Only works for Run or Idle graphic types.
     */
    Rotation::Direction toDirection(EntityGraphicType graphicType);

    /**
     * Converts a Direction to the associated Run graphic type.
     */
    EntityGraphicType toRunGraphicType(Rotation::Direction direction);

    /**
     * If graphicType is an Idle graphic type, converts it to the Run graphic 
     * type of the same direction. If it's already a Run graphic type, does 
     * nothing.
     * Only works for Run or Idle graphic types.
     */
    EntityGraphicType toRunGraphicType(EntityGraphicType graphicType);

    /**
     * Converts a Direction to the associated Idle graphic type.
     */
    EntityGraphicType toIdleGraphicType(Rotation::Direction direction);

    /**
     * If graphicType is a Run graphic type, converts it to the Idle graphic 
     * type of the same direction. If it's already an Idle graphic type, does 
     * nothing.
     * Only works for Run or Idle graphic types.
     */
    EntityGraphicType toIdleGraphicType(EntityGraphicType graphicType);

    /** Used to get entity GraphicState components. */
    World& world;

    /** Used to get entity graphics. */
    GraphicData& graphicData;

    /** If non-nullptr, contains the project's simulation extension functions.
        Used to give the project a chance to update entity graphics. */
    ISimulationExtension* extension;
};

} // End namespace Client
} // End namespace AM
