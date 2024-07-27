#include "GraphicID.h"

namespace AM
{

bool isSpriteID(GraphicID graphicID)
{
    return !((static_cast<Uint32>(graphicID) & GRAPHIC_ID_TYPE_MASK) >> 31);
}

bool isAnimationID(GraphicID graphicID)
{
    return ((static_cast<Uint32>(graphicID) & GRAPHIC_ID_TYPE_MASK) >> 31);
}

SpriteID toSpriteID(GraphicID graphicID)
{
    if (!isSpriteID(graphicID)) {
        return NULL_SPRITE_ID;
    }

    return (static_cast<SpriteID>(graphicID) & GRAPHIC_ID_VALUE_MASK);
}

AnimationID toAnimationID(GraphicID graphicID)
{
    if (!isAnimationID(graphicID)) {
        return NULL_ANIMATION_ID;
    }

    return (static_cast<AnimationID>(graphicID) & GRAPHIC_ID_VALUE_MASK);
}

GraphicID toGraphicID(SpriteID spriteID)
{
    // Sprite IDs don't need any special changes.
    return static_cast<GraphicID>(spriteID);
}

GraphicID toGraphicID(AnimationID animationID)
{
    // Set the top bit to indicate that this is an animation.
    return static_cast<GraphicID>(animationID | GRAPHIC_ID_ANIMATION_VALUE);
}

} // End namespace AM
