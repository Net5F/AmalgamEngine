#pragma once

#include "EditorSprite.h"
#include "EditorFloorSpriteSet.h"
#include "EditorFloorCoveringSpriteSet.h"
#include "EditorWallSpriteSet.h"
#include "EditorObjectSpriteSet.h"
#include "EditorIcon.h"
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
    = std::variant<std::monostate, EditorSprite, EditorFloorSpriteSet,
                   EditorFloorCoveringSpriteSet, EditorWallSpriteSet,
                   EditorObjectSpriteSet, EditorIcon>;

} // End namespace ResourceImporter
} // End namespace AM
