#pragma once

#include "SpriteSets.h"
#include "Wall.h"
#include "Rotation.h"
#include <concepts>

namespace AM
{

struct TileLayer {
    /** The types of layers that our tiles support. */
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
        floor layer, this will be set to nullptr if this layer has no floor. */
    const FloorSpriteSet* spriteSet{nullptr};

    // Floors currently only support a single sprite, so no enum is needed.
    // Eventually, we may add support for "variations" of a floor, so we could
    // support a randomized floor placement tool in build mode.

    /** Note: May be nullptr if this layer has no floor. */
    const Sprite* getSprite() const { 
        if (spriteSet == nullptr) {
            return nullptr;
        }
        else {
            return &(spriteSet->sprite);
        }
    }
};

struct FloorCoveringTileLayer {
    /** This floor covering's sprite set. */
    const FloorCoveringSpriteSet* spriteSet{nullptr};

    /** The direction that this floor covering is facing. */
    Rotation::Direction direction{Rotation::Direction::None};

    /** Note: May be nullptr if there's no sprite for the current direction,
              but generally you should avoid setting directions that have 
              no sprite. */
    const Sprite* getSprite() const
    {
        return spriteSet->sprites[direction];
    }
};

struct WallTileLayer {
    /** This wall's sprite set. Since each tile always has 2 stack-allocated 
        wall layers, this will be set to nullptr if this layer has no wall. */
    const WallSpriteSet* spriteSet{nullptr};

    /** The type of wall that this is. This will be set to None if the tile 
        has no wall. */
    Wall::Type wallType{Wall::Type::None};

    /** Note: May be nullptr if this layer has no wall. */
    const Sprite* getSprite() const
    {
        if (spriteSet == nullptr) {
            return nullptr;
        }
        else {
            return &(spriteSet->sprites[wallType].get());
        }
    };
};

struct ObjectTileLayer {
    /** This object's sprite set. */
    const ObjectSpriteSet* spriteSet{nullptr};

    /** The direction that this object is facing. */
    Rotation::Direction direction{Rotation::Direction::None};

    /** Note: May be nullptr if there's no sprite for the current direction,
              but generally you should avoid setting directions that have 
              no sprite. */
    const Sprite* getSprite() const
    {
        return spriteSet->sprites[direction];
    };
};

/** Concept to match the tile layer types. */
template<typename... T>
concept IsTileLayerType = ((std::same_as<T, FloorTileLayer>
        || std::same_as<T, FloorCoveringTileLayer>
        || std::same_as<T, WallTileLayer> || std::same_as<T, ObjectTileLayer>)
        || ...);

} // End namespace AM
