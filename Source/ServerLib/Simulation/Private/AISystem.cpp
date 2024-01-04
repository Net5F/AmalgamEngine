#include "AISystem.h"
#include "World.h"
#include "ProjectAITypes.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"

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
    // For each AI type in the list, update all AI components of that type.
    boost::mp11::mp_for_each<ProjectAITypes>(
        [&](auto I) { 
            using AIType = decltype(I);

            auto view{world.registry.view<AIType>()};
            for (auto [entity, aiLogic] : view.each()) {
                aiLogic.tick(world, entity);
            }
        });
}

} // End namespace Server
} // End namespace AM
