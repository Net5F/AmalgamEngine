#pragma once

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
        Floors currently only support 1 graphic, but support for "variations"
        may be added in the future. */
    std::array<GraphicID, 1> graphicIDs{NULL_GRAPHIC_ID};
};

} // namespace ResourceImporter
} // namespace AM
