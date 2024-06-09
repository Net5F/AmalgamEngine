#pragma once

#include "Terrain.h"
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
 * Holds the data necessary for editing and saving a terrain graphic set.
 * Part of GraphicSetModel.
 */
struct EditorTerrainGraphicSet {
    /** This graphic set's unique numeric identifier.
        Note: This ID may change when this graphic set is saved to the json. */
    TerrainGraphicSetID numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each graphic in this set.
        A full terrain set has 37 graphic types, but fewer can be used if 
        desired (e.g. not using 1/4 and 3/4 heights). If a graphic isn't 
        provided, it should be set to NULL_GRAPHIC_ID. */
    std::array<GraphicID, Terrain::Type::Count> graphicIDs{
        /* NULL_GRAPHIC_ID */ };
};

} // namespace ResourceImporter
} // namespace AM
