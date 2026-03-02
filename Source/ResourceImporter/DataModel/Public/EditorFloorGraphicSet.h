#pragma once

#include "Rotation.h"
#include "GraphicID.h"
#include "GraphicSetIDs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving a floor graphic set.
 * Part of GraphicSetModel.
 */
struct EditorFloorGraphicSet {
    /** This graphic set's unique numeric identifier.
        Note: This ID may change when this graphic set is saved to the json. */
    FloorGraphicSetID numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each graphic in this set.
        Floors support 8 directions of rotation. At least 1 graphic
        must be set. If a direction isn't provided, it should be set to
        NULL_GRAPHIC_ID. */
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{
        /* NULL_GRAPHIC_ID */ };
};

} // namespace ResourceImporter
} // namespace AM
