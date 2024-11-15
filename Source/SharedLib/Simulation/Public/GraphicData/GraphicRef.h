#pragma once

#include "Sprite.h"
#include "Animation.h"
#include "GraphicID.h"
#include <variant>

namespace AM
{
/**
 * Holds a reference to either a Sprite or an Animation.
 *
 * Used in situations where both are supported (graphic sets, tiles, entities).
 */
struct GraphicRef : public std::variant<std::reference_wrapper<const Sprite>,
                                        std::reference_wrapper<const Animation>>
{
    /**
     * Returns the graphic ID for the Sprite or Animation that this ref points 
     * to.
     * 
     * The top bit is this graphic's type. 0 == Sprite, 1 == Animation.
     * The bottom 31 bits are the Sprite or Animation's ID.
     */
    GraphicID getGraphicID() const;

    const std::string& getDisplayName() const;

    bool getCollisionEnabled() const;

    const BoundingBox& getModelBounds() const;

    /**
     * If this ref points to a Sprite, returns it.
     * If this ref points to an Animation, returns the first sprite in the 
     * animation.
     */
    const Sprite& getFirstSprite() const;

    /**
     * If this ref points to a Sprite, returns it.
     * If this ref points to an Animation, returns the sprite that should be 
     * displayed at the given animation time.
     */
    const Sprite& getSpriteAtTime(double animationTime) const;
};

} // End namespace AM
