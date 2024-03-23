#pragma once

#include "EditorSprite.h"
#include "EditorAnimation.h"
#include "EditorFloorGraphicSet.h"
#include "EditorFloorCoveringGraphicSet.h"
#include "EditorWallGraphicSet.h"
#include "EditorObjectGraphicSet.h"
#include "EditorEntityGraphicSet.h"
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
 *
 * Note: These pointers should never be null, you don't need to check them. 
 *       We just use them because std::reference_wrapper is 
 */
using LibraryItemData
    = std::variant<std::monostate, std::reference_wrapper<const EditorSprite>,
                   std::reference_wrapper<const EditorAnimation>,
                   std::reference_wrapper<const EditorFloorGraphicSet>,
                   std::reference_wrapper<const EditorFloorCoveringGraphicSet>,
                   std::reference_wrapper<const EditorWallGraphicSet>,
                   std::reference_wrapper<const EditorObjectGraphicSet>,
                   std::reference_wrapper<const EditorEntityGraphicSet>,
                   std::reference_wrapper<const EditorIcon>,
                   std::reference_wrapper<const EditorBoundingBox>>;

// Note: We define these convenience functions because variant<reference_wrapper>
//       is very noisy otherwise.
template<typename T>
bool holds_alternative(const LibraryItemData& libraryItemData)
{
    return std::holds_alternative<std::reference_wrapper<const T>>(
        libraryItemData);
}

template <typename T>
const T& get(const LibraryItemData& libraryItemData)
{
    return std::get<std::reference_wrapper<const T>>(libraryItemData).get();
}

template <typename T>
const T* get_if(const LibraryItemData* libraryItemData)
{
    const auto* dataPtr{
        std::get_if<std::reference_wrapper<const T>>(libraryItemData)};
    if (dataPtr) {
        return &(dataPtr->get());
    }
    else {
        return nullptr;
    }
}

} // End namespace ResourceImporter
} // End namespace AM
