#pragma once

#include "BoundingBoxID.h"
#include "BoundingBox.h"
#include <string>

namespace AM
{
namespace ResourceImporter
{

/**
 * Holds the data necessary for editing and saving a bounding box.
 * Part of BoundingBoxModel.
 */
struct EditorBoundingBox {
    /** This bounding box's unique numeric identifier. */
    BoundingBoxID numericID{NULL_BOUNDING_BOX_ID};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** Model-space bounding box. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace ResourceImporter
} // namespace AM
