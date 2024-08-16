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
     * Adds the given layer to this tile.
     */
    void addLayer(const TileOffset& tileOffset, TileLayer::Type layerType,
                  const GraphicSet& graphicSet, Uint8 graphicValue);

    /**
     * Removes any layers with a matching offset, type, graphic index, and
     * graphic set.
     *
     * @return true if the tile had any matching layers to remove, else false.
     */
    std::size_t removeLayers(const TileOffset& tileOffset,
                             TileLayer::Type layerType, Uint16 graphicSetID,
                             Uint8 graphicValue);

    /**
     * Removes any layers with a matching type, graphic index, and graphic set.
     *
     * @return true if the tile had any matching layers to remove, else false.
     */
    std::size_t removeLayers(TileLayer::Type layerType, Uint16 graphicSetID,
                             Uint8 graphicValue);

    /**
     * Removes any layers with a matching type and graphic index, regardless 
     * of their graphic set or offset.
     *
     * @return The number of matching layers that were removed.
     */
    std::size_t removeLayers(TileLayer::Type layerType, Uint8 graphicValue);

    /**
     * Clears all layers of the given types from this tile.
     *
     * @return The number of matching layers that were cleared.
     */
    std::size_t clearLayers(
        const std::array<bool, TileLayer::Type::Count>& layerTypesToRemove);

    /**
     * Clears all of this tile's layers.
     *
     * @return The number of layers that were cleared.
     */
    std::size_t clear();

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
    TileLayer* findLayer(TileLayer::Type layerType, Uint8 graphicValue);
    const TileLayer* findLayer(TileLayer::Type layerType,
                               Uint8 graphicValue) const;
    TileLayer* findLayer(TileLayer::Type layerType);
    const TileLayer* findLayer(TileLayer::Type layerType) const;

    /** Returns true if this tile has no layers, else false. */
    bool isEmpty() const;

private:
    // TODO: Maybe eventually switch to an alternative vector type that
    //       has a smaller footprint but only supports forward iterators.
    /** The graphic layers that are on this tile, sorted by their 
        TileLayer::Type in increasing order. */
    std::vector<TileLayer> layers{};
};

} // End namespace AM
