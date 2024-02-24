#pragma once

#include "EditorSprite.h"
#include "EditorAnimation.h"
#include "GraphicID.h"
#include <variant>

namespace AM
{
namespace ResourceImporter
{

class BoundingBoxModel;

/**
 * Holds a reference to either an EditorSprite or an EditorAnimation.
 *
 * Used in situations where both are supported (graphic sets).
 */
struct EditorGraphicRef
: public std::variant<std::reference_wrapper<const EditorSprite>,
                      std::reference_wrapper<const EditorAnimation>> {
    /**
     * Returns the graphic ID for the Sprite or Animation that this ref points 
     * to.
     * 
     * The top bit is this graphic's type. 0 == Sprite, 1 == Animation.
     * The bottom 31 bits are the Sprite or Animation's ID.
     */
    GraphicID getGraphicID() const;

    bool getCollisionEnabled() const;

    const BoundingBox&
        getModelBounds(const BoundingBoxModel& boundingBoxModel) const;

    /**
     * If this ref points to a Sprite, returns it.
     * If this ref points to an Animation, returns the first sprite in the 
     * animation.
     */
    const EditorSprite& getFirstSprite() const;

    /**
     * If this ref points to a Sprite, returns it.
     * If this ref points to an Animation, returns the sprite that should be 
     * displayed at the given animation time.
     */
    const EditorSprite& getSpriteAtTime(double animationTime) const;
};

} // End namespace ResourceImporter
} // End namespace AM
