#include "Tile.h"
#include "GraphicSets.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include <algorithm>

namespace AM
{

const std::vector<BoundingBox>& Tile::getCollisionBoxes() const
{
    return collisionBoxes;
}

void Tile::addLayer(TileLayer::Type layerType, const GraphicSet& graphicSet,
                    Uint8 graphicIndex)
{
    // Insert the new layer, being careful to keep the vector sorted.
    TileLayer newLayer{layerType, graphicIndex, graphicSet};
    layers.insert(
        std::lower_bound(layers.begin(), layers.end(), newLayer,
                         [](const TileLayer& layer, const TileLayer& newLayer) {
                             return layer.type < newLayer.type;
                         }),
        newLayer);
}

bool Tile::removeLayer(TileLayer::Type layerType, Uint16 graphicSetID,
                       Uint8 graphicIndex)
{
    // Erase any layers with a matching type, graphic index, and graphic set.
    std::size_t erasedCount{std::erase_if(layers, [&](const TileLayer& layer) {
        if ((layer.type == layerType) && (layer.graphicIndex == graphicIndex)
            && (layer.graphicSet.get().numericID == graphicSetID)) {
            return true;
        }
        return false;
    })};

    return (erasedCount > 0);
}

bool Tile::removeLayers(TileLayer::Type layerType, Uint8 graphicIndex)
{
    // Erase any layers with a matching type and graphic index.
    std::size_t erasedCount{std::erase_if(layers, [&](const TileLayer& layer) {
        if ((layer.type == layerType) && (layer.graphicIndex == graphicIndex)) {
            return true;
        }
        return false;
    })};

    return (erasedCount > 0);
}

bool Tile::clearLayers(
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    // Erase any layers with a matching type.
    std::size_t numErased{std::erase_if(layers, [&](const TileLayer& layer) {
        return layerTypesToClear[layer.type];
    })};

    return (numErased > 0);
}

bool Tile::clear()
{
    bool notEmpty{layers.size() > 0};

    layers.clear();

    return notEmpty;
}

std::span<TileLayer> Tile::getLayers(TileLayer::Type layerType)
{
    auto begin{layers.end()};
    auto end{layers.end()};
    for (auto it{layers.begin()}; it != layers.end(); ++it) {
        if (it->type == layerType) {
            // If this is the first match, set our iterators.
            if (begin == layers.end()) {
                begin = it;
                end = it + 1;
            }
            else {
                // Found another match, move our end iterator forward.
                end = it + 1;
            }
        }
        else if (it->type == (layerType + 1)) {
            // Reached a type past the one we were looking for, stop looking.
            break;
        }
    }

    // If we found any matches, return them.
    if (begin != layers.end()) {
        return {begin, end};
    }
    else {
        return {};
    }
}

std::span<const TileLayer> Tile::getLayers(TileLayer::Type layerType) const
{
    auto begin{layers.end()};
    auto end{layers.end()};
    for (auto it{layers.begin()}; it != layers.end(); ++it) {
        if (it->type == layerType) {
            // If this is the first match, set our iterators.
            if (begin == layers.end()) {
                begin = it;
                end = it + 1;
            }
            else {
                // Found another match, move our end iterator forward.
                end = it + 1;
            }
        }
        else if (it->type == (layerType + 1)) {
            // Reached a type past the one we were looking for, stop looking.
            break;
        }
    }

    // If we found any matches, return them.
    if (begin != layers.end()) {
        return {begin, end};
    }
    else {
        return {};
    }
}

std::vector<TileLayer>& Tile::getAllLayers()
{
    return layers;
}

const std::vector<TileLayer>& Tile::getAllLayers() const
{
    return layers;
}

TileLayer* Tile::findLayer(TileLayer::Type type, Uint8 graphicIndex)
{
    for (TileLayer& layer : layers) {
        if ((layer.type == type)
            && (layer.graphicIndex == graphicIndex)) {
            return &layer;
        }
        else if (layer.type == (type + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

const TileLayer* Tile::findLayer(TileLayer::Type type, Uint8 graphicIndex) const
{
    for (const TileLayer& layer : layers) {
        if ((layer.type == type)
            && (layer.graphicIndex == graphicIndex)) {
            return &layer;
        }
        else if (layer.type == (type + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

TileLayer* Tile::findLayer(TileLayer::Type type)
{
    for (TileLayer& layer : layers) {
        if (layer.type == type) {
            return &layer;
        }
        else if (layer.type == (type + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

const TileLayer* Tile::findLayer(TileLayer::Type type) const
{
    for (const TileLayer& layer : layers) {
        if (layer.type == type) {
            return &layer;
        }
        else if (layer.type == (type + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

void Tile::rebuildCollision(const TilePosition& tilePosition)
{
    // Clear out the old collision boxes.
    collisionBoxes.clear();

    // Add all of this tile's layers that have collision.
    for (const TileLayer& layer : layers) {
        // Skip floor coverings (they never have collision).
        if (layer.type == TileLayer::Type::FloorCovering) {
            continue;
        }

        GraphicRef graphic{layer.getGraphic()};
        if (graphic.getCollisionEnabled()) {
            collisionBoxes.push_back(
                calcWorldBoundsForGraphic(tilePosition, graphic));
        }
    }
}

BoundingBox Tile::calcWorldBoundsForGraphic(const TilePosition& tilePosition,
                                            const GraphicRef& graphic)
{
    // Cast constants to a float so we get float multiplication below.
    static constexpr float TILE_WORLD_WIDTH{SharedConfig::TILE_WORLD_WIDTH};
    static constexpr float TILE_WORLD_HEIGHT{SharedConfig::TILE_WORLD_HEIGHT};

    Position position{tilePosition.x * TILE_WORLD_WIDTH,
                      tilePosition.y * TILE_WORLD_WIDTH,
                      tilePosition.z * TILE_WORLD_HEIGHT};
    return Transforms::modelToWorld(graphic.getModelBounds(), position);
}

} // End namespace AM
