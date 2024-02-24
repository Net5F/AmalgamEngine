#pragma once

#include "SpriteID.h"
#include "AnimationID.h"
#include <SDL_stdinc.h>

namespace AM
{

/** A graphic's numeric ID.
    The top bit is the graphic's type. 0 == Sprite, 1 == Animation.
    The bottom 31 bits are the Sprite or Animation's ID. */
using GraphicID = Uint32;

/** The mask to use when getting a graphic ID's type. */
static constexpr Uint32 GRAPHIC_ID_TYPE_MASK{0x80000000};

/** The mask to use when getting a graphic ID's value. */
static constexpr Uint32 GRAPHIC_ID_VALUE_MASK{0x7FFFFFFF};

/** The value used to indicate that this graphic ID holds an animation. */
static constexpr Uint32 GRAPHIC_ID_ANIMATION_VALUE{0x80000000};

/**
 * Returns true if the given graphic ID is for a sprite. Else, returns false.
 */
static bool isSpriteID(GraphicID graphicID)
{
    return !((static_cast<Uint32>(graphicID) & GRAPHIC_ID_TYPE_MASK) >> 31);
}

/**
 * Returns true if the given graphic ID is for an animation. Else, returns false.
 */
static bool isAnimationID(GraphicID graphicID)
{
    return ((static_cast<Uint32>(graphicID) & GRAPHIC_ID_TYPE_MASK) >> 31);
}

static SpriteID toSpriteID(GraphicID graphicID)
{
    if (!isSpriteID(graphicID)) {
        return NULL_SPRITE_ID;
    }

    return (static_cast<SpriteID>(graphicID) & GRAPHIC_ID_VALUE_MASK);
}

static AnimationID toAnimationID(GraphicID graphicID)
{
    if (!isAnimationID(graphicID)) {
        return NULL_ANIMATION_ID;
    }

    return (static_cast<AnimationID>(graphicID) & GRAPHIC_ID_VALUE_MASK);
}

static GraphicID toGraphicID(SpriteID spriteID)
{
    // Sprite IDs don't need any special changes.
    return static_cast<GraphicID>(spriteID);
}

static GraphicID toGraphicID(AnimationID animationID)
{
    // Set the top bit to indicate that this is an animation.
    return static_cast<GraphicID>(animationID | GRAPHIC_ID_ANIMATION_VALUE);
}

/**
 * The ID of the "null graphic", or the ID used to indicate that a graphic 
 * is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (graphicID)".
 */
static constexpr GraphicID NULL_GRAPHIC_ID{0};

} // End namespace AM
