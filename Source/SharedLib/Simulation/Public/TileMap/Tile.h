#pragma once

#include "BoundingBox.h"
#include "TileLayer.h"
#include "TilePosition.h"
#include <vector>
#include <span>

namespace AM
{
struct Sprite;

/**
 * A tile in the tile map.
 *
 * Tiles consist of layers of sprites, which can be floors, walls, etc.
 *
 * Tiles contain no logic. If something on a tile requires logic, e.g. a tree
 * growing over time, it must have a system act upon it.
 *
 * Tiles can have the following layer counts:
 *   1 floor
 *   Any number of floor coverings
 *   2 walls
 *   Any number of objects
 * All layers are optional and may not be present in a given tile.
 */
class Tile
{
public:
    /**
     * Returns the collision boxes of each of this tile's layers.
     * Note: The returned vector may be empty, if this tile has no collision.
     */
    const std::vector<BoundingBox>& getCollisionBoxes() const;

    /**
     * Adds the given layer to this tile.
     */
    void addLayer(TileLayer::Type layerType, const GraphicSet& graphicSet,
                  Uint8 graphicIndex);

    /**
     * Removes any layers with a matching type, graphic index, and graphic set.
     *
     * Note: This function is named singularly even though it may remove 
     *       multiple layers, because typically there will only be one layer 
     *       that matches all 3 values.
     *
     * @return true if the tile had any matching layers to remove, else false.
     */
    bool removeLayer(TileLayer::Type layerType, Uint16 graphicSetID,
                     Uint8 graphicIndex);

    /**
     * Removes any layers with a matching type and graphic index, regardless 
     * of their graphic set.
     *
     * @return true if the tile had any matching layers to remove, else false.
     */
    bool removeLayers(TileLayer::Type layerType, Uint8 graphicIndex);

    /**
     * Clears all layers of the given types from this tile.
     *
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clearLayers(
        const std::array<bool, TileLayer::Type::Count>& layerTypesToRemove);

    /**
     * Clears all of this tile's layers.
     *
     * @return true if any layers were cleared. false if the tile was empty.
     */
    bool clear();

    /**
     * @return This tile's layers of the given type, if it has any.
     * Note: This span will be invalidated if you add, remove, or clear any 
     *       of this tile's layers.
     */
    std::span<TileLayer> getLayers(TileLayer::Type layerType);
    std::span<const TileLayer> getLayers(TileLayer::Type layerType) const;

    /**
     * @return All of this tile's layers.
     */
    std::vector<TileLayer>& getAllLayers();
    const std::vector<TileLayer>& getAllLayers() const;

    /**
     * Returns a pointer to the first matching layer in this tile. If one isn't 
     * found, returns nullptr.
     */
    TileLayer* findLayer(TileLayer::Type type, Uint8 graphicIndex);
    const TileLayer* findLayer(TileLayer::Type type, Uint8 graphicIndex) const;
    TileLayer* findLayer(TileLayer::Type type);
    const TileLayer* findLayer(TileLayer::Type type) const;

    /**
     * Clears the collisionBoxes vector, then refills it with all of this
     * tile's walls and objects.
     *
     * @param tilePosition This tile's world coordinates.
     */
    void rebuildCollision(const TilePosition& tilePosition);

private:
    /**
     * Returns the given graphic's modelBounds, translated to world space and
     * offset to the given tile coords.
     */
    BoundingBox calcWorldBoundsForGraphic(const TilePosition& tilePosition,
                                          const GraphicRef& graphic);

    /** Holds this tile's collision boxes.
        We pre-calculate these and store them contiguously to speed up collision
        checking. */
    std::vector<BoundingBox> collisionBoxes{};

    // TODO: Maybe eventually switch to an alternative vector type that
    //       has a smaller footprint but only supports forward iterators.
    /** The graphic layers that are on this tile, sorted by their 
        TileLayer::Type in increasing order. */
    std::vector<TileLayer> layers{};
};

} // End namespace AM
