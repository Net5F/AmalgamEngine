#pragma once

#include "AccountDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <vector>

namespace AM
{

/**
 * A player entity's PlayerPersistence::Account components, as persisted by
 * the account server.
 *
 * The account server treats these as opaque blobs. Only the world server
 * knows how to read them (see WorldServer's PersistedComponentList.h and
 * PERSISTED_COMPONENTS_VERSION).
 *
 * Note: Engine and project components use separate ID namespaces, so their
 *       record lists are kept in separate blobs.
 */
struct AccountData {
    /** Ordered tagged engine component records. */
    std::vector<Uint8> engineComponents{};

    /** Ordered tagged project component records. */
    std::vector<Uint8> projectComponents{};

    /**
     * Returns true if both blobs are within ACCOUNT_DATA_BLOB_MAX_BYTES.
     * Data that fails this check can't be serialized.
     */
    bool isWithinSizeLimits() const
    {
        return (engineComponents.size() <= ACCOUNT_DATA_BLOB_MAX_BYTES)
               && (projectComponents.size() <= ACCOUNT_DATA_BLOB_MAX_BYTES);
    }
};

template<typename S>
void serialize(S& serializer, AccountData& accountData)
{
    serializer.container1b(accountData.engineComponents,
                           ACCOUNT_DATA_BLOB_MAX_BYTES);
    serializer.container1b(accountData.projectComponents,
                           ACCOUNT_DATA_BLOB_MAX_BYTES);
}

} // End namespace AM
