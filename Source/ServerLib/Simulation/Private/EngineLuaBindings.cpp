#include "EngineLuaBindings.h"
#include "World.h"
#include "sol/sol.hpp"

namespace AM
{
namespace Server
{

EngineLuaBindings::EngineLuaBindings(sol::state& inEntityInitLua,
                                     sol::state& inItemInitLua, World& inWorld)
: entityInitLua{inEntityInitLua}
, itemInitLua{inItemInitLua}
, world{inWorld}
{
}

void EngineLuaBindings::addBindings()
{
}

} // namespace Server
} // namespace AM
