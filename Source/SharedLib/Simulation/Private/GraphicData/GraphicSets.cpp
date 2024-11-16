#include "GraphicSets.h"
#include "Vector3.h"
#include "VariantTools.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{

BoundingBox EntityGraphicSet::getCollisionModelBounds() const
{
    AM_ASSERT(graphics.contains(EntityGraphicType::IdleSouth),
              "Entity graphic set is missing IdleSouth: %s.",
              displayName.c_str());

    const GraphicRef& idleSouthGraphicRef{
        graphics.at(EntityGraphicType::IdleSouth)};
    return idleSouthGraphicRef.getModelBounds();
}

Vector3 EntityGraphicSet::getRenderAlignmentOffset(
    EntityGraphicType graphicType) const
{
    if (!graphics.contains(EntityGraphicType::IdleSouth)) {
        LOG_ERROR("Entity graphic set is missing IdleSouth: %s.",
                  displayName.c_str());
        return {};
    }
    if (!graphics.contains(graphicType)) {
        LOG_ERROR("Entity graphic set is missing requested type: %s, %u.",
                  displayName.c_str(), graphicType);
        return {};
    }

    // If the requested graphic is a Sprite, return 0 (sprites don't have 
    // alignment anchors).
    const GraphicRef& graphicRef{graphics.at(graphicType)};
    if (std::holds_alternative<std::reference_wrapper<const Sprite>>(
            graphicRef)) {
        return {};
    }

    // If the requested Animation doesn't have an alignment anchor, return 0.
    const auto& animation{
        std::get<std::reference_wrapper<const Animation>>(graphicRef)};
    if (!(animation.get().entityAlignmentAnchor)) {
        return {};
    }

    // Return the difference between the requested animation's alignment 
    // anchor and the IdleSouth graphic's bottom center.
    Vector3 idleSouthBottomCenter{graphics.at(EntityGraphicType::IdleSouth)
                                      .getModelBounds()
                                      .getBottomCenterPoint()};
    Vector3 alignmentAnchor{animation.get().entityAlignmentAnchor.value()};
    return (alignmentAnchor - idleSouthBottomCenter);
}

} // End namespace AM
