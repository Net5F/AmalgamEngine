#include "GraphicSets.h"
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

} // End namespace AM
