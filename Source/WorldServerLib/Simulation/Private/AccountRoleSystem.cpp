#include "AccountRoleSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "AccountRole.h"

namespace AM
{
namespace WorldServer
{

AccountRoleSystem::AccountRoleSystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
, roleChangedNoticeQueue{inSimContext.networkEventDispatcher}
{
}

void AccountRoleSystem::processRoleChanges()
{
    AccountRoleChangedNotice notice{};
    while (roleChangedNoticeQueue.pop(notice)) {
        entt::entity entity{world.getAccountEntity(notice.accountID)};
        if (entity == entt::null) {
            // The account isn't in the world. Nothing to do.
            continue;
        }

        world.registry.patch<AccountRole>(
            entity, [&](AccountRole& accountRole) {
                accountRole.roleID = notice.roleID;
            });
    }
}

} // End namespace WorldServer
} // End namespace AM
