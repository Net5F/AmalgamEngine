#pragma once

#include <SDL3/SDL_stdinc.h>

namespace AM
{
namespace WorldServer
{

/**
 * Info about a client entity that was just loaded, passed to
 * ISimulationExtension::initClientEntity().
 *
 * The flags tell you which situation the account is in:
 *   Neither: A brand new account.
 *   Account only: The account's first time on this world.
 *   World only: A returning account, in a setup with no Account components
 *               (e.g. single server or world-locked characters).
 *   Both: A returning account.
 *
 * To check which components were loaded, check the registry directly.
 */
struct ClientEntityLoadInfo {
    /** The account that the entity belongs to. */
    Sint64 accountID{0};

    /** True if the account had saved ClientPersistence::Account data, i.e. it
        has played on some world before.
        Always false if no components are set to Account. */
    bool hadAccountData{false};

    /** True if the account had saved ClientPersistence::World data on this
        world, i.e. it has played on this world before.
        Always false if no components are set to World. */
    bool hadWorldData{false};
};

} // namespace WorldServer
} // namespace AM
