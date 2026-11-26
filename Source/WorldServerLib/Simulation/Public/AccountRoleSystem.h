#pragma once

#include "AccountRoleChangedNotice.h"
#include "QueuedEvents.h"

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;

/**
 * Applies AccountServer's notices that an account's role was changed, by
 * updating the AccountRole of that account's player entity.
 *
 * Note: AccountRole is replicated, so the change also reaches the client.
 */
class AccountRoleSystem
{
public:
    AccountRoleSystem(const SimulationContext& inSimContext);

    /**
     * Processes any role change notices that AccountServer has sent us.
     */
    void processRoleChanges();

private:
    /** Used to access the player entities. */
    World& world;
    EventQueue<AccountRoleChangedNotice> roleChangedNoticeQueue;
};

} // End namespace WorldServer
} // End namespace AM
