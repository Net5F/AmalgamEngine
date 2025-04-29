#pragma once

#include "GraphicID.h"
#include "GraphicSetIDs.h"
#include "EntityGraphicType.h"
#include "Rotation.h"
#include <SDL_stdinc.h>
#include <string>
#include <unordered_map>
#include <array>

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

    /** The entity graphic types that this set contains.
        Each graphic type is associated with an array of up to 8 graphics, one 
        for each possible rotation of the entity.
        The Idle graphic type will always be present. All others are optional.
        If any slots weren't assigned a graphic, they will be set to 
        NULL_GRAPHIC_ID in the array. */
    std::unordered_map<EntityGraphicType,
                       std::array<GraphicID, Rotation::Direction::Count>>
        graphicIDs{};
};

} // namespace ResourceImporter
} // namespace AM
