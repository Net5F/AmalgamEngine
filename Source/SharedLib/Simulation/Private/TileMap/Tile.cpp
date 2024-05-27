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
    std::size_t numErased{0};
    for (auto it{layers.begin()}; it != layers.end();) {
        TileLayer& layer{*it};

        // If this layer matches, erase it.
        if ((layer.type == layerType) && (layer.graphicIndex == graphicIndex)
            && (layer.graphicSet.get().numericID == graphicSetID)) {
            it = layers.erase(it);
            numErased++;
        }
        // If we've reached a type past the desired one, stop looking.
        else if (layer.type == (layerType + 1)) {
            break;
        }
        else {
            it++;
        }
    }

    return (numErased > 0);
}

bool Tile::removeLayers(TileLayer::Type layerType, Uint8 graphicIndex)
{
    // Erase any layers with a matching type and graphic index.
    std::size_t numErased{0};
    for (auto it{layers.begin()}; it != layers.end();) {
        TileLayer& layer{*it};

        // If this layer matches, erase it.
        if ((layer.type == layerType) && (layer.graphicIndex == graphicIndex)) {
            it = layers.erase(it);
            numErased++;
        }
        // If we've reached a type past the desired one, stop looking.
        else if (layer.type == (layerType + 1)) {
            break;
        }
        else {
            it++;
        }
    }

    return (numErased > 0);
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

TileLayer* Tile::findLayer(TileLayer::Type layerType, Uint8 graphicIndex)
{
    for (TileLayer& layer : layers) {
        if ((layer.type == layerType)
            && (layer.graphicIndex == graphicIndex)) {
            return &layer;
        }
        else if (layer.type == (layerType + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

const TileLayer* Tile::findLayer(TileLayer::Type layerType,
                                 Uint8 graphicIndex) const
{
    for (const TileLayer& layer : layers) {
        if ((layer.type == layerType)
            && (layer.graphicIndex == graphicIndex)) {
            return &layer;
        }
        else if (layer.type == (layerType + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

TileLayer* Tile::findLayer(TileLayer::Type layerType)
{
    for (TileLayer& layer : layers) {
        if (layer.type == layerType) {
            return &layer;
        }
        else if (layer.type == (layerType + 1)) {
            // We've reached a type past the desired one, stop looking.
            return nullptr;
        }
    }

    return nullptr;
}

const TileLayer* Tile::findLayer(TileLayer::Type layerType) const
{
    for (const TileLayer& layer : layers) {
        if (layer.type == layerType) {
            return &layer;
        }
        else if (layer.type == (layerType + 1)) {
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
        GraphicRef graphic{layer.getGraphic()};

        // If it's a floor covering, skip it (they never have collision).
        if (layer.type == TileLayer::Type::FloorCovering) {
            continue;
        }
        // If it's a floor, generate a collision plane for it.
        // (We ignore modelBounds and collisionEnabled on floor graphics, all 
        // floors get a generated collision plane). 
        else if (layer.type == TileLayer::Type::Floor) {
            collisionBoxes.push_back(getFloorWorldBounds(tilePosition));
        }
        // If it's a wall or object, add its assigned collision.
        else if (graphic.getCollisionEnabled()) {
            collisionBoxes.push_back(
                calcWorldBoundsForGraphic(tilePosition, graphic));
        }
    }
}

BoundingBox Tile::getFloorWorldBounds(const TilePosition& tilePosition)
{
    static constexpr float TILE_WORLD_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};

    Position tileOrigin{tilePosition.getOriginPosition()};
    return {tileOrigin.x, tileOrigin.x + TILE_WORLD_WIDTH,
            tileOrigin.y, tileOrigin.y + TILE_WORLD_WIDTH,
            tileOrigin.z, tileOrigin.z};
}

bool Tile::isEmpty() const
{
    return layers.empty();
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
