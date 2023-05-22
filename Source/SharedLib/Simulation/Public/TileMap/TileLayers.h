#pragma once

#include "SpriteSets.h"
#include "Wall.h"
#include "Rotation.h"

namespace AM
{

struct TileLayer {
    /** The type of layers that our tiles support. */
    enum Type : Uint8 { 
        Floor,
        /** Floor coverings are things like rugs, flowers, puddles, etc. */
        FloorCovering,
        Wall,
        Object,
        Count,
        None
    };
};

struct FloorTileLayer {
    /** This floor's sprite set. Since each tile always has a stack-allocated 
        floor layer, this will be set to nullptr if the tile has no floor. */
    const FloorSpriteSet* spriteSet{nullptr};

    // Floors currently only support a single sprite, so no enum is needed.
    // Eventually, we may add support for "variations" of a floor, so we could
    // support a randomized floor placement tool in build mode.
    const Sprite* getSprite() const { return spriteSet->sprite; }
};

struct FloorCoveringTileLayer {
    /** This floor covering's sprite set. Should always be non-nullptr, since 
        empty floor covering layers get removed. */
    const FloorCoveringSpriteSet* spriteSet{nullptr};

    /** The direction that this floor covering is facing. */
    Rotation::Direction direction{Rotation::Direction::None};

    const Sprite* getSprite() const
    {
        return spriteSet->sprites[direction];
    }
};

struct WallTileLayer {
    /** This wall's sprite set. Since each tile always has 2 stack-allocated 
        wall layers, this will be set to nullptr if the tile has no wall. */
    const WallSpriteSet* spriteSet{nullptr};

    /** The type of wall that this is. This will be set to None if the tile 
        has no wall. */
    Wall::Type wallType{Wall::Type::None};

    const Sprite* getSprite() const
    {
        return spriteSet->sprites[wallType];
    };
};

struct ObjectTileLayer {
    /** This object's sprite set. Should always be non-nullptr, since empty 
        object layers get removed. */
    const ObjectSpriteSet* spriteSet{nullptr};

    /** The direction that this object is facing. */
    Rotation::Direction direction{Rotation::Direction::None};

    const Sprite* getSprite() const
    {
        return spriteSet->sprites[direction];
    };
};

} // End namespace AM
