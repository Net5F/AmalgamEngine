#pragma once

#include "GraphicSetIDs.h"
#include "GraphicRef.h"
#include "Wall.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>
#include <functional>
#include <optional>

namespace AM
{

/**
 * Base class for a single graphic set from ResourceData.json.
 *
 * Graphic sets are used for grouping graphic into a higher-level concept. A
 * FloorCoveringGraphicSet, for example, holds all of the available rotations
 * of a single floor covering, such as a rug.
 *
 * Floor, FloorCovering, and Wall are only used for tiles.
 * Object is used for tiles (static objects) and entities (dynamic objects).
 * Character is only used for entities.
 */
struct GraphicSet {
    enum Type : Uint8 {
        Floor,
        /** Floor coverings are things like rugs, flowers, puddles, etc. */
        FloorCovering,
        Wall,
        Object,
        // Entity,
        Count,
        None
    };

    /** Unique display name, shown in the UI.  */
    std::string displayName{"Empty"};

    /** The graphic set's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"empty"};
};

struct FloorGraphicSet : public GraphicSet {
    /** This graphic set's unique numeric identifier.
        Each graphic set type has its own ID space.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when ResourceData.json is
        modified. */
    FloorGraphicSetID numericID{0};

    /** This floor's single graphic. */
    GraphicRef graphic;
};

struct FloorCoveringGraphicSet : public GraphicSet {
    /** The number of variations that this graphic set can hold. */
    static constexpr std::size_t VARIATION_COUNT{8};

    /** See FloorGraphicSet::numericID */
    FloorCoveringGraphicSetID numericID{0};

    /** The 8 variations of this floor covering.
        If any slots weren't assigned a graphic, they will be set to the null 
        sprite in this array (check for NULL_GRAPHIC_ID).
        When useful, the graphics in this array can represent the same floor
        covering, facing different directions. In such a case, you can set  
        these elements equal to a Rotation::Direction. */
    std::array<GraphicRef, VARIATION_COUNT> graphics;
};

struct WallGraphicSet : public GraphicSet {
    /** See FloorGraphicSet::numericID */
    WallGraphicSetID numericID{0};

    /** The 4 types of wall graphic that we use for our modular wall system. */
    std::array<GraphicRef, Wall::Type::Count> graphics;
};

struct ObjectGraphicSet : public GraphicSet {
    /** The number of variations that this graphic set can hold. */
    static constexpr std::size_t VARIATION_COUNT{8};

    /** See FloorGraphicSet::numericID */
    ObjectGraphicSetID numericID{0};

    /** See FloorCoveringGraphicSet::graphics. */
    std::array<GraphicRef, VARIATION_COUNT> graphics;
};

} // namespace AM
