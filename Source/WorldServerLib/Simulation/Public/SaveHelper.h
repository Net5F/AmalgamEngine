#pragma once

#include "PersistedComponentList.h"
#include "BinaryBuffer.h"
#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <unordered_map>

namespace AM
{
namespace WorldServer
{
class World;
class Simulation;

/**
 * Helper class for saving persisted entity state.
 *
 * Note: Saves go to the in-memory database, which is only backed up to the
 *       file database periodically (see SaveSystem). The exception is client
 *       entity ClientPersistence::Account components, which are queued to be
 *       saved to the account server (see AccountSaveQueue).
 *
 * If a client entity's World components fail to save, we keep the serialized
 * data and retry each time client entities are saved. Until it succeeds, the
 * data is used if the account logs back in (see getUnsavedWorldData()).
 */
class SaveHelper
{
public:
    /** A client entity's serialized ClientPersistence::World components. */
    struct ClientWorldData {
        /** Ordered tagged engine component records. */
        BinaryBuffer engineComponents{};
        /** Ordered tagged project component records. */
        BinaryBuffer projectComponents{};
    };

    SaveHelper(World& inWorld, Simulation& inSimulation);

    /**
     * Saves all non-client entities to the in-memory database.
     */
    void saveNonClientEntities();

    /**
     * Saves all client entities (including lingering ones), then retries any
     * World saves that previously failed.
     */
    void saveClientEntities();

    /**
     * Saves the given client entity's persisted components, split by
     * ClientPersistence. World components are saved to the in-memory
     * database, Account components are queued to be saved to the account
     * server.
     *
     * Call this before removing a client entity from the world, so its latest
     * state is what gets loaded when it reconnects.
     */
    void saveClientEntity(entt::entity entity);

    /**
     * Returns the given account's World data, if its last save failed and
     * hasn't been successfully retried yet. If there isn't a pending save for
     * the given account, returns nullptr.
     *
     * When an account logs in, this data is newer than what's in the
     * database, so it should be used instead.
     */
    const ClientWorldData* getUnsavedWorldData(Sint64 accountID) const;

private:
    /**
     * Queues the given client entity's ClientPersistence::Account components
     * to be saved to the account server. Does nothing if no components use
     * Account.
     */
    void saveAccountComponents(entt::entity entity, Sint64 accountID);

    /**
     * Saves the given client entity's ClientPersistence::World components to
     * the in-memory database. Does nothing if no components use World.
     *
     * If the save fails, adds the data to failedWorldSaves.
     * If the data exceeds CLIENT_WORLD_DATA_BLOB_MAX_BYTES, skips the save,
     * keeping whatever data was previously saved.
     */
    void saveWorldComponents(entt::entity entity, Sint64 accountID);

    /**
     * Retries each save in failedWorldSaves, removing the ones that succeed.
     */
    void retryFailedWorldSaves();

    /**
     * Serializes each component from EntryList that entity possesses into
     * outputBuffer, replacing its contents.
     *
     * @tparam EntryList A list of persisted component entries, e.g.
     *                   EnginePersistedComponentTypes::EntryTypes or
     *                   EnginePersistedComponentTypes::ClientEntryTypes<...>.
     *
     * Note: PersistedComponentList requires its entries to be ordered by
     *       ascending ID, so the serialized records will also be ordered.
     */
    template<typename EntryList>
    void serializeComponents(entt::entity entity, BinaryBuffer& outputBuffer);

    /**
     * Appends component to outputBuffer using the persisted component record
     * format.
     *
     * See PERSISTED_COMPONENTS_VERSION comment for format info.
     */
    template<typename Entry>
    void serializeComponent(typename Entry::Component& component,
                            BinaryBuffer& outputBuffer);

    /** Used to access the registry and database. */
    World& world;
    /** Used to get the current tick. */
    Simulation& simulation;

    /** Scratch buffers used while serializing components. */
    BinaryBuffer workBuffer1;
    BinaryBuffer workBuffer2;

    /** Maps account IDs -> their World data that failed to save. */
    std::unordered_map<Sint64, ClientWorldData> failedWorldSaves;
};

} // namespace WorldServer
} // namespace AM
