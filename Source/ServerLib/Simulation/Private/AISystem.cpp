#include "AISystem.h"
#include "World.h"
#include "AIBehavior.h"
#include "Log.h"

namespace AM
{
namespace Server
{

AISystem::AISystem(World& inWorld)
: world{inWorld}
{
}

void AISystem::processAITick()
{
    auto view{world.registry.view<AIBehavior>()};
    for (auto [entity, aiBehavior] : view.each()) {
        aiBehavior.logic->tick();
    }
}

} // End namespace Server
} // End namespace AM
