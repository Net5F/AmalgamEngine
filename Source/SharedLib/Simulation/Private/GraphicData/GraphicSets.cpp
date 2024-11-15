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
              "All entity graphic sets must contain IdleSouth.");

    const GraphicRef& idleSouthGraphicRef{
        graphics.at(EntityGraphicType::IdleSouth)};
    return idleSouthGraphicRef.getModelBounds();
}

Vector3 EntityGraphicSet::getRenderAlignmentOffset(
    EntityGraphicType graphicType) const
{
    AM_ASSERT(graphics.contains(graphicType),
              "Requested a graphic type that was not present in the set.");

    const GraphicRef& graphicRef{graphics.at(graphicType)};
    if (const auto* animation{
            std::get_if<std::reference_wrapper<const Animation>>(
                &graphicRef)}) {
        return animation->get().entityAlignmentOffset;
    }

    return {};
}

} // End namespace AM
