#pragma once

#include "Sprite.h"
#include "Wall.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>
#include <functional>

namespace AM
{

/**
 * Base class for a single sprite set from ResourceData.json.
 *
 * Sprite sets are used for grouping sprites into a higher-level concept. A
 * FloorCoveringSpriteSet, for example, holds all of the available rotations
 * of a single floor covering, such as a rug.
 *
 * Floor, FloorCovering, and Wall are only used for tiles.
 * Object is used for tiles (static objects) and entities (dynamic objects).
 * Character is only used for entities.
 */
struct SpriteSet {
    enum Type : Uint8 {
        Floor,
        /** Floor coverings are things like rugs, flowers, puddles, etc. */
        FloorCovering,
        Wall,
        Object,
        // Character,
        Count,
        None
    };

    /** Unique display name, shown in the UI.  */
    std::string displayName{"Empty"};

    /** The sprite set's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"empty"};

    /** This sprite set's unique numeric identifier.
        Each sprite set type has its own ID space.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when ResourceData.json is
        modified. */
    Uint16 numericID{0};
};

struct FloorSpriteSet : public SpriteSet {
    /** This floor's single sprite. */
    const Sprite& sprite;
};

struct FloorCoveringSpriteSet : public SpriteSet {
    /** The number of variations that this sprite set can hold. */
    static constexpr std::size_t VARIATION_COUNT{8};

    /** The 8 variations of this floor covering.
        If any slots weren't assigned a sprite, they will be nullptr in
        this array.
        When useful, the sprites in this array can represent the same floor
        covering, facing different directions. In such a case, you can use
        Rotation::directionToIndex() to match an index to a direction. */
    std::array<const Sprite*, VARIATION_COUNT> sprites{};
};

struct WallSpriteSet : public SpriteSet {
    /** The 4 types of wall sprite that we use for our modular wall system. */
    std::array<std::reference_wrapper<const Sprite>, Wall::Type::Count> sprites;
};

struct ObjectSpriteSet : public SpriteSet {
    /** The number of variations that this sprite set can hold. */
    static constexpr std::size_t VARIATION_COUNT{8};

    /** The 8 variations of this floor covering.
        If any slots weren't assigned a sprite, they will be nullptr in
        this array.
        When useful, the sprites in this array can represent the same floor
        covering, facing different directions. In such a case, you can use
        Rotation::directionToIndex() to match an index to a direction. */
    std::array<const Sprite*, VARIATION_COUNT> sprites{};
};

} // namespace AM
