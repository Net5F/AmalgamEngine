#pragma once

#include "entt/signal/sigh.hpp"

#include "ConnectionError.h"
#include "TileExtent.h"
#include "Position.h"

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
    /** We've established a connection with the server and the simulation has
        started running. */
    entt::sigh<void()> simulationStarted;

    /** Our connection to the server has encountered an error. */
    entt::sigh<void(ConnectionError)> serverConnectionError;

    /** The tile map's extent (size) has changed. */
    entt::sigh<void(TileExtent)> tileMapExtentChanged;

    /** The player entity's position has changed. */
    entt::sigh<void(Position)> playerPositionChanged;
};

} // namespace Client
} // namespace AM
