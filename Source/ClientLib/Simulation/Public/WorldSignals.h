#pragma once

#include "entt/signal/sigh.hpp"

#include "TileExtent.h"

namespace AM
{
namespace Client
{
/**
 * Owns the World's signals.
 *
 * Used by the sim's systems to push world state updates out to the UI.
 *
 * The UI's WorldSinks object will subscribe to these signals.
 */
class WorldSignals
{
public:
    /** The tile map's extent (size) has changed. */
    entt::sigh<void(TileExtent)> tileMapExtentChanged;
};

} // namespace Client
} // namespace AM
