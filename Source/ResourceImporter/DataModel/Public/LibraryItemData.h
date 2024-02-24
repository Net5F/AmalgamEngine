#pragma once

#include "EditorSprite.h"
#include "EditorFloorGraphicSet.h"
#include "EditorFloorCoveringGraphicSet.h"
#include "EditorWallGraphicSet.h"
#include "EditorObjectGraphicSet.h"
#include "EditorIcon.h"
#include "EditorBoundingBox.h"
#include <variant>

namespace AM
{
namespace ResourceImporter
{

/**
 * A generic way to reference the data associated with a LibraryListItem.
 *
 * This only includes the types that the user is able to double click to load
 * into the edit stage.
 */
using LibraryItemData
    = std::variant<std::monostate, EditorSprite, EditorFloorGraphicSet,
                   EditorFloorCoveringGraphicSet, EditorWallGraphicSet,
                   EditorObjectGraphicSet, EditorIcon, EditorBoundingBox>;

} // End namespace ResourceImporter
} // End namespace AM
