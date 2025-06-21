#pragma once

#include "TileLayerID.h"
#include "AVEntityID.h"
#include "entt/fwd.hpp"
#include <variant>

namespace AM
{
namespace Client
{

/**
 * A variant used when we need to generically identify an object from the
 * simulation's World.
 *
 * Useful, e.g., when returning the object that the user clicked on.
 *
 * Note: "Object" doesn't mean the same as when we say "Object tile layer".
 *       A better name for this would be welcome.
 */
using WorldObjectID
    = std::variant<std::monostate, TileLayerID, entt::entity, AVEntityID>;

} // namespace Client
} // namespace AM
