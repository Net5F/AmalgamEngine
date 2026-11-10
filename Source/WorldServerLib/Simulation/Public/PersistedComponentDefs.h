#pragma once

#include "AccountDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <cstddef>

namespace AM
{
// Note: Persisted components are server-only.
namespace WorldServer
{

/**
 * The version number of the outer serialization format used for persisted
 * components.
 *
 * Our current format is a contiguous, ordered array of tagged component 
 * records. Each record looks like:
 *   [typeID][version][payloadSize][payload (serialized component data)]
 *
 * The persisted component type lists can be modified without changing this.
 * This only needs to be incremented if the outer serialization format changes.
 */
inline constexpr unsigned int PERSISTED_COMPONENTS_VERSION{0};

/** Offsets and sizes related to serialization of persisted components. */
inline constexpr std::size_t TYPE_ID_OFFSET{0};
inline constexpr std::size_t VERSION_OFFSET{2};
inline constexpr std::size_t PAYLOAD_SIZE_OFFSET{4};
inline constexpr std::size_t COMPONENT_HEADER_SIZE{8};

/**
 * The max size of each of a player entity's PlayerPersistence::World blobs
 * (engine and project components), in bytes.
 *
 * World blobs are saved to this world's database, so nothing forces a limit.
 * We match the Account limit so that any component set can be moved from
 * World to Account persistence without exceeding it. It also catches
 * components that grow without bound.
 *
 * Note: Non-player entities aren't limited, since they never move to Account
 *       persistence.
 */
inline constexpr std::size_t CLIENT_WORLD_DATA_BLOB_MAX_BYTES{
    ACCOUNT_DATA_BLOB_MAX_BYTES};

} // namespace WorldServer
} // namespace AM
