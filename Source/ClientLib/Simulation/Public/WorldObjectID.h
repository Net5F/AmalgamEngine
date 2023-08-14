#pragma once

#include "TileLayerID.h"
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
 * Note: "Object" doesn't mean the same as when we say "Static Object" or 
 *       "Dynamic Object". A better name for this would be welcome.
 */
using WorldObjectID = std::variant<std::monostate, TileLayerID, entt::entity>;

} // namespace Client
} // namespace AM
