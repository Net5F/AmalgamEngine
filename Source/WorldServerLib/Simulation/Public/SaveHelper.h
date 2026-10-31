#pragma once

#include "PersistedComponentList.h"
#include "BinaryBuffer.h"
#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>

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
 *       file database periodically (see SaveSystem).
 */
class SaveHelper
{
public:
    SaveHelper(World& inWorld, Simulation& inSimulation);

    /**
     * Saves all non-client entities to the in-memory database.
     */
    void saveNonClientEntities();

    /**
     * Saves all client entities (including lingering ones) to the in-memory
     * database.
     */
    void saveClientEntities();

    /**
     * Saves the given client entity's persisted components to the in-memory
     * database, split by ClientPersistence.
     *
     * Call this before removing a client entity from the world, so its latest
     * state is what gets loaded when it reconnects.
     */
    void saveClientEntity(entt::entity entity);

private:
    /**
     * Saves the given client entity's components that use the given
     * persistence. Does nothing if no components use it.
     */
    template<ClientPersistence Persistence>
    void saveClientComponents(entt::entity entity, Sint64 accountID);

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
};

} // namespace WorldServer
} // namespace AM
