#include "GraphicSets.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{

const Sprite& TerrainGraphicSet::getThumbnailSprite() const
{
    for (const GraphicRef& graphic : graphics) {
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            return graphic.getFirstSprite();
        }
    }

    return graphics[0].getFirstSprite();
}

const Sprite& FloorGraphicSet::getThumbnailSprite() const
{
    for (const GraphicRef& graphic : graphics) {
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            return graphic.getFirstSprite();
        }
    }

    return graphics[0].getFirstSprite();
}

const Sprite& WallGraphicSet::getThumbnailSprite() const
{
    return graphics[0].getFirstSprite();
}

const Sprite& ObjectGraphicSet::getThumbnailSprite() const
{
    for (const GraphicRef& graphic : graphics) {
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            return graphic.getFirstSprite();
        }
    }

    return graphics[0].getFirstSprite();
}

const Sprite& EntityGraphicSet::getThumbnailSprite() const
{
    // Note: Idle South is guaranteed to be present in every entity graphic
    //       set (though it may be the null sprite).
    const auto& graphicArr{graphics.at(EntityGraphicType::Idle)};
    return graphicArr.at(Rotation::Direction::South).getFirstSprite();
}

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
