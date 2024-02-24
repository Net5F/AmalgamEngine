#pragma once

#include "GraphicSets.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Represents an entity's graphic state.
 * 
 * The entity's collision box and mouse hit detection area (in build mode) are 
 * derived from this state.
 */
struct GraphicState
{
    /** The type of graphic set that this entity uses. */
    GraphicSet::Type graphicSetType{GraphicSet::Type::None};

    /** The numeric ID of this entity's graphic set. */
    Uint16 graphicSetID{0};

    // TODO: (Do this?) On the Server, this only ever goes between Idle, 
    //       Crouching, and Jumping (the only states where Collision is 
    //       changed). On the Client, this may go between any of the graphics.
    /** The index within graphicSet.graphics of this entity's current graphic. */
    Uint8 graphicIndex{0};
};

template<typename S>
void serialize(S& serializer, GraphicState& graphicState)
{
    serializer.value1b(graphicState.graphicSetType);
    serializer.value2b(graphicState.graphicSetID);
    serializer.value1b(graphicState.graphicIndex);
}

} // namespace AM
