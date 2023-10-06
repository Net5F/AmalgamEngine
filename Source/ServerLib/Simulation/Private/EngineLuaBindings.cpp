#include "EngineLuaBindings.h"
#include "World.h"
#include "sol/sol.hpp"

namespace AM
{
namespace Server
{

EngineLuaBindings::EngineLuaBindings(sol::state& inLua, World& inWorld)
: lua{inLua}
, world{inWorld}
{
}

void EngineLuaBindings::addBindings()
{
}

} // namespace Server
} // namespace AM
