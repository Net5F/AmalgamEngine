#pragma once

#include "Wall.h"
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
struct EditorWallGraphicSet {
    /** This graphic set's unique numeric identifier.
        Note: This ID may change when this graphic set is saved to the json. */
    WallGraphicSetID numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each graphic in this set.
        Walls require the 4 types of wall graphics that our modular wall
        system uses. */
    std::array<GraphicID, Wall::Type::Count> graphicIDs{
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID};
};

} // namespace ResourceImporter
} // namespace AM
