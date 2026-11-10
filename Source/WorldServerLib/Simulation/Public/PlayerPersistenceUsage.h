#pragma once

#include "PersistedComponentList.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"

namespace AM
{
namespace WorldServer
{

/**
 * True if any engine or project components use the given PlayerPersistence.
 *
 * If no components use a persistence, there's nothing to save or load for it,
 * so we can skip it entirely. For example, if nothing uses Account, we don't
 * ask the account server for AccountData.
 */
template<PlayerPersistence Persistence>
inline constexpr bool USES_PLAYER_PERSISTENCE{
    EnginePersistedComponentTypes::HAS_PLAYER_ENTRIES<Persistence>
    || ProjectPersistedComponentTypes::HAS_PLAYER_ENTRIES<Persistence>};

} // namespace WorldServer
} // namespace AM
