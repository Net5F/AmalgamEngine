#pragma once

#include "entt/signal/sigh.hpp"

#include "TileExtent.h"

namespace AM
{
namespace Client
{
class WorldSignals;

/**
 * Owns sinks for the World's signals.
 *
 * Allows the UI to connect to the World's signals to listen for updates.
 */
class WorldSinks
{
public:
    WorldSinks(WorldSignals& worldSignals);

    /** The tile map's extent (size) has changed. */
    entt::sink<void(TileExtent)> tileMapExtentChanged;
};

} // namespace Client
} // namespace AM
