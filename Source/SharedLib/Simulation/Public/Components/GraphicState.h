#pragma once

#include "GraphicSets.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Holds graphic-related state that is shared between the client and server.
 *
 * For client-specific graphic state, see ClientGraphicState.
 *
 * The entity's collision box and mouse hit detection area (in build mode) are 
 * derived from this state.
 */
struct GraphicState
{
    EntityGraphicSetID graphicSetID{NULL_ENTITY_GRAPHIC_SET_ID};
};

template<typename S>
void serialize(S& serializer, GraphicState& graphicState)
{
    serializer.value2b(graphicState.graphicSetID);
}

} // namespace AM
