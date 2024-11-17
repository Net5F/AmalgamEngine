#pragma once

#include "GraphicSetIDs.h"
#include "GraphicRef.h"
#include "EntityGraphicType.h"
#include "Terrain.h"
#include "Wall.h"
#include <SDL_stdinc.h>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <optional>

namespace AM
{

/**
 * Base class for a single graphic set from ResourceData.json.
 *
 * Graphic sets are used for grouping graphic into a higher-level concept. A
 * FloorGraphicSet, for example, holds all of the available rotations of a 
 * single floor covering, such as a rug.
 *
 * Terrain, Floor, Wall, and Object are only used for tiles.
 * Entity is only used for entities.
 */
struct GraphicSet {
    enum Type : Uint8 {
        Terrain,
        /** Floors are display-only, they have no collision. They're used for 
            things like grass, carpets, flooring, etc. */
        Floor,
        Wall,
        Object,
        Entity,
        Count,
        None
    };

    /** Unique display name, shown in the UI.  */
    std::string displayName{"Empty"};

    /** The graphic set's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"empty"};

    /** This graphic set's unique numeric identifier.
        Can be cast to FloorGraphicSetID, etc depending on this set's type.
        Each graphic set type has its own ID space.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when ResourceData.json is
        modified. */
    Uint16 numericID{0};
};

struct TerrainGraphicSet : public GraphicSet {
    /** The 4 heights of terrain graphic that we use for our terrain system. */
    std::array<GraphicRef, Terrain::Height::Count> graphics;
};

struct FloorGraphicSet : public GraphicSet {
    /** The number of variations that this graphic set can hold. */
    static constexpr std::size_t VARIATION_COUNT{8};

    /** The 8 variations of this floor covering.
        If any slots weren't assigned a graphic, they will be set to the null 
        sprite in this array (check for NULL_GRAPHIC_ID).
        When useful, the graphics in this array can represent the same floor
        covering, facing different directions. In such a case, you can set  
        these elements equal to a Rotation::Direction. */
    std::array<GraphicRef, VARIATION_COUNT> graphics;
};

struct WallGraphicSet : public GraphicSet {
    /** The 4 types of wall graphic that we use for our modular wall system. */
    std::array<GraphicRef, Wall::Type::Count> graphics;
};

struct ObjectGraphicSet : public GraphicSet {
    /** The number of variations that this graphic set can hold. */
    static constexpr std::size_t VARIATION_COUNT{8};

    /** See FloorCoveringGraphicSet::graphics. */
    std::array<GraphicRef, VARIATION_COUNT> graphics;
};

struct EntityGraphicSet : public GraphicSet {
    /** The types of entity graphic that this set contains.
        The IdleSouth graphic type will always be present. All others are 
        optional. */
    std::unordered_map<EntityGraphicType, GraphicRef> graphics;

    /**
     * Returns the IdleSouth graphic's model-space collision bounds.
     * 
     * We use IdleSouth's bounds for all collision in order to keep collision 
     * behavior consistent.
     */
    BoundingBox getCollisionModelBounds() const;
};

} // namespace AM
