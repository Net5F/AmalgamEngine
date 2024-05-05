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
    return const_cast<Tile*>(this)->getLayers(layerType);
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
    return const_cast<Tile*>(this)->findLayer(type, graphicIndex);
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
    return const_cast<Tile*>(this)->findLayer(type);
}

void Tile::rebuildCollision(int tileX, int tileY)
{
    // Clear out the old collision boxes.
    collisionBoxes.clear();

    // Add all of this tile's walls.
    for (const TileLayer& wallLayer : getLayers(TileLayer::Type::Wall)) {
        GraphicRef graphic{wallLayer.getGraphic()};
        if (graphic.getCollisionEnabled()) {
            collisionBoxes.push_back(
                calcWorldBoundsForGraphic(tileX, tileY, graphic));
        }
    }

    // Add all of this tile's objects.
    for (const TileLayer& objectLayer : getLayers(TileLayer::Type::Object)) {
        GraphicRef graphic{objectLayer.getGraphic()};
        if (graphic.getCollisionEnabled()) {
            collisionBoxes.push_back(
                calcWorldBoundsForGraphic(tileX, tileY, graphic));
        }
    }
}

BoundingBox Tile::calcWorldBoundsForGraphic(int tileX, int tileY,
                                            const GraphicRef& graphic)
{
    Position tilePosition{
        static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
        static_cast<float>(tileY * SharedConfig::TILE_WORLD_WIDTH), 0};
    return Transforms::modelToWorld(graphic.getModelBounds(), tilePosition);
}

} // End namespace AM
