#include "EditorSprite.h"
#include "BoundingBoxModel.h"

namespace AM
{
namespace ResourceImporter
{

const BoundingBox&
    EditorSprite::getModelBounds(const BoundingBoxModel& boundingBoxModel) const
{
    if (modelBoundsID) {
        return boundingBoxModel.getBoundingBox(modelBoundsID).modelBounds;
    }
    else {
        return customModelBounds;
    }
}

} // End namespace ResourceImporter
} // End namespace AM
