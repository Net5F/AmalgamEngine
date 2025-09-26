#include "GraphicSets.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{

BoundingBox EntityGraphicSet::getCollisionModelBounds() const
{
    AM_ASSERT(graphics.contains(EntityGraphicType::Idle),
              "Entity graphic set is missing Idle: %s.", displayName.c_str());

    // Note: Entity collision always comes from its Idle South graphic.
    const auto& idleGraphicArray{graphics.at(EntityGraphicType::Idle)};
    const GraphicRef& idleSouthGraphicRef{
        idleGraphicArray[Rotation::Direction::South]};
    return idleSouthGraphicRef.getModelBounds();
}

bool EntityGraphicSet::contains(EntityGraphicType type,
                                Rotation::Direction direction) const
{
    auto it{graphics.find(type)};
    if (it != graphics.end()) {
        return (it->second.at(direction).getGraphicID() != NULL_GRAPHIC_ID);
    }

    return false;
}

} // End namespace AM
