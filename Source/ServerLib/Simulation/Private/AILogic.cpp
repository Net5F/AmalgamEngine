#include "AILogic.h"
#include "World.h"

namespace AM
{
namespace Server
{

AILogic::AILogic(World& inWorld, entt::entity inEntity)
: world{inWorld}
, entity{inEntity}
{
}

} // namespace Server
} // namespace AM
