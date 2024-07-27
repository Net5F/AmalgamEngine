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
bool isSpriteID(GraphicID graphicID);

/**
 * Returns true if the given graphic ID is for an animation. Else, returns false.
 */
bool isAnimationID(GraphicID graphicID);

/**
 * Converts the given graphic ID into a sprite ID.
 */
SpriteID toSpriteID(GraphicID graphicID);

/**
 * Converts the given graphic ID into a animation ID.
 */
AnimationID toAnimationID(GraphicID graphicID);

/**
 * Converts the given sprite ID into a graphic ID.
 */
GraphicID toGraphicID(SpriteID spriteID);

/**
 * Converts the given animation ID into a graphic ID.
 */
GraphicID toGraphicID(AnimationID animationID);

/**
 * The ID of the "null graphic", or the ID used to indicate that a graphic 
 * is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (graphicID)".
 */
static constexpr GraphicID NULL_GRAPHIC_ID{0};

} // End namespace AM
