#include "AISystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "ProjectAITypes.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"

namespace AM
{
namespace Server
{

AISystem::AISystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
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
