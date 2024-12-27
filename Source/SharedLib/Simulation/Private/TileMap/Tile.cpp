#include "Tile.h"
#include "GraphicSets.h"
#include "SharedConfig.h"
#include <algorithm>

namespace AM
{

void Tile::addLayer(const TileOffset& tileOffset, TileLayer::Type layerType,
                    const GraphicSet& graphicSet, Uint8 graphicValue)
{
    if (layers.size() == UINT8_MAX) {
        LOG_INFO("Failed to add layer: limit reached.");
        return;
    }

    // Insert the new layer, being careful to keep the vector sorted.
    TileLayer newLayer{tileOffset, layerType, graphicValue, graphicSet};
    layers.insert(
        std::lower_bound(layers.begin(), layers.end(), newLayer,
                         [](const TileLayer& layer, const TileLayer& newLayer) {
                             return layer.type < newLayer.type;
                         }),
        newLayer);
}

std::size_t Tile::removeLayers(const TileOffset& tileOffset, TileLayer::Type layerType,
                       Uint16 graphicSetID, Uint8 graphicValue)
{
    // Erase any layers with a matching type, graphic index, and graphic set.
    std::size_t numErased{0};
    for (auto it{layers.begin()}; it != layers.end();) {
        TileLayer& layer{*it};

        // If this layer matches, erase it.
        if ((layer.tileOffset == tileOffset) && (layer.type == layerType)
            && (layer.graphicValue == graphicValue)
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

    return numErased;
}

std::size_t Tile::removeLayers(TileLayer::Type layerType, Uint16 graphicSetID,
                               Uint8 graphicValue)
{
    // Erase any layers with a matching type, graphic index, and graphic set.
    std::size_t numErased{0};
    for (auto it{layers.begin()}; it != layers.end();) {
        TileLayer& layer{*it};

        // If this layer matches, erase it.
        if ((layer.type == layerType) && (layer.graphicValue == graphicValue)
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

    return numErased;
}

std::size_t Tile::removeLayers(TileLayer::Type layerType, Uint8 graphicValue)
{
    // Erase any layers with a matching type and graphic index.
    std::size_t numErased{0};
    for (auto it{layers.begin()}; it != layers.end();) {
        TileLayer& layer{*it};

        // If this layer matches, erase it.
        if ((layer.type == layerType) && (layer.graphicValue == graphicValue)) {
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

    return numErased;
}

std::size_t Tile::clearLayers(
    const std::array<bool, TileLayer::Type::Count>& layerTypesToClear)
{
    // Erase any layers with a matching type.
    std::size_t numErased{std::erase_if(layers, [&](const TileLayer& layer) {
        return layerTypesToClear[layer.type];
    })};

    return numErased;
}

std::size_t Tile::clear()
{
    std::size_t layerCount{layers.size()};

    layers.clear();

    return layerCount;
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

TileLayer* Tile::findLayer(TileLayer::Type layerType, Uint8 graphicValue)
{
    for (TileLayer& layer : layers) {
        if ((layer.type == layerType)
            && (layer.graphicValue == graphicValue)) {
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
                                 Uint8 graphicValue) const
{
    for (const TileLayer& layer : layers) {
        if ((layer.type == layerType)
            && (layer.graphicValue == graphicValue)) {
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

bool Tile::isEmpty() const
{
    return layers.empty();
}

} // End namespace AM
