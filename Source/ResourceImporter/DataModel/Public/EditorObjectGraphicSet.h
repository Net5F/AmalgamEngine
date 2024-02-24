#pragma once

#include "Rotation.h"
#include "GraphicID.h"
#include "GraphicSetIDs.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving an object graphic set.
 * Part of GraphicSetModel.
 */
struct EditorObjectGraphicSet {
    /** This graphic set's unique numeric identifier.
        Note: This ID may change when this graphic set is saved to the json. */
    ObjectGraphicSetID numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each graphic in this set.
        Objects support 8 directions of rotation. At least 1 graphic must be
        set. If a direction isn't provided, it should be set to
        NULL_GRAPHIC_ID. */
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID,
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID};
};

} // namespace ResourceImporter
} // namespace AM
