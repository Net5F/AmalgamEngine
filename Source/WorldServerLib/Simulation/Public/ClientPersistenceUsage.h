#pragma once

#include "PersistedComponentList.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"

namespace AM
{
namespace WorldServer
{

/**
 * True if any engine or project components use the given ClientPersistence.
 *
 * If no components use a persistence, there's nothing to save or load for it,
 * so we can skip it entirely. For example, if nothing uses Account, we don't
 * ask the account server for AccountData.
 */
template<ClientPersistence Persistence>
inline constexpr bool USES_CLIENT_PERSISTENCE{
    EnginePersistedComponentTypes::HAS_CLIENT_ENTRIES<Persistence>
    || ProjectPersistedComponentTypes::HAS_CLIENT_ENTRIES<Persistence>};

} // namespace WorldServer
} // namespace AM
