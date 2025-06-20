#pragma once

#include "EntityGraphicType.h"
#include "entt/fwd.hpp"

namespace AM
{
namespace Client
{

class World;
class GraphicData;
class ISimulationExtension;

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
     * Returns the desired graphic type for the given entity, based on its 
     * current state.
     */
    EntityGraphicType getDesiredGraphicType(entt::entity entity);

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
