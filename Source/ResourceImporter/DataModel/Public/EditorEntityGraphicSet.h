#pragma once

#include "GraphicID.h"
#include "GraphicSetIDs.h"
#include "EntityGraphicType.h"
#include <SDL_stdinc.h>
#include <string>
#include <unordered_map>

namespace AM
{
namespace ResourceImporter
{
/**
 * Holds the data necessary for editing and saving an entity graphic set.
 * Part of EntityGraphicSetModel.
 */
struct EditorEntityGraphicSet {
    /** This graphic set's unique numeric identifier.
        Note: This ID may change when this graphic set is saved to the json. */
    EntityGraphicSetID numericID{0};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The numeric IDs for each graphic in this set.
        The Idle graphic type will always be present. All others are optional. */
    std::unordered_map<EntityGraphicType, GraphicID> graphicIDs{};
};

} // namespace ResourceImporter
} // namespace AM
