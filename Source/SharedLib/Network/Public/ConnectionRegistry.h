#pragma once

#include "ConnectionHandle.h"
#include "IDPool.h"
#include "Log.h"
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace AM
{
/**
 * Owns the current connections for one endpoint.
 *
 * This type is intentionally not synchronized. Endpoint code must access it
 * only from its network io_context.
 */
template<typename ConnectionType>
class ConnectionRegistry
{
public:
    explicit ConnectionRegistry(std::size_t inCapacity)
    : capacity{getCapacity(inCapacity)}
    , connectionIDPool{IDPool::ReservationStrategy::MarchForward, capacity + 1}
    , entries(capacity + 1)
    , connectionCount{0}
    {
        // ID 0 is reserved so a default-constructed handle is always null.
        connectionIDPool.markIDAsReserved(0);
    }

    ~ConnectionRegistry() { closeAll(); }

    ConnectionRegistry(const ConnectionRegistry&) = delete;
    ConnectionRegistry& operator=(const ConnectionRegistry&) = delete;

    /**
     * Attempts to add the given connection to this registry.
     *
     * If successful, returns a handle. If not, returns null.
     */
    std::optional<ConnectionHandle>
        add(std::shared_ptr<ConnectionType> connection)
    {
        if (!connection || full()) {
            return std::nullopt;
        }

        ConnectionHandle::ID id{static_cast<ConnectionHandle::ID>(
            connectionIDPool.reserveID())};
        Entry& entry{entries[id]};
        if (entry.connection) {
            LOG_ERROR("Reserved connection ID is already in use: %u",
                      static_cast<unsigned int>(id));
            connectionIDPool.freeID(id);
            return std::nullopt;
        }

        entry.connection = std::move(connection);
        ++connectionCount;
        return ConnectionHandle{id, entry.generation};
    }

    /**
     * If the given handle matches a valid connection, returns it. Otherwise
     * returns nullptr.
     */
    std::shared_ptr<ConnectionType> find(ConnectionHandle handle) const
    {
        if (!handle || (handle.id() >= entries.size())) {
            return {};
        }

        const Entry& entry{entries[handle.id()]};
        if (!entry.connection
            || (entry.generation != handle.generation())) {
            return {};
        }
        return entry.connection;
    }

    /**
     * Attempts to delete the given connection from this registry.
     * @return true if erased, else false (connection not found).
     */
    bool erase(ConnectionHandle handle)
    {
        if (!handle || (handle.id() >= entries.size())) {
            return false;
        }

        Entry& entry{entries[handle.id()]};
        if (!entry.connection
            || (entry.generation != handle.generation())) {
            return false;
        }

        std::shared_ptr<ConnectionType> connection{
            std::move(entry.connection)};
        entry.generation = nextGeneration(entry.generation);
        connectionIDPool.freeID(handle.id());
        --connectionCount;

        connection->close();
        return true;
    }

    /**
     * Closes all connections in this registry.
     */
    void closeAll()
    {
        std::vector<std::shared_ptr<ConnectionType>> connections{};
        connections.reserve(connectionCount);

        for (std::size_t id{1}; id < entries.size(); ++id) {
            Entry& entry{entries[id]};
            if (!entry.connection) {
                continue;
            }

            connections.push_back(std::move(entry.connection));
            entry.generation = nextGeneration(entry.generation);
            connectionIDPool.freeID(static_cast<unsigned int>(id));
        }
        connectionCount = 0;

        for (const std::shared_ptr<ConnectionType>& connection : connections) {
            connection->close();
        }
    }

    /**
     * Returns the number of connections in this registry.
     */
    std::size_t size() const { return connectionCount; }

    /**
     * Returns true if this registry is empty, else false.
     */
    bool empty() const { return connectionCount == 0; }

    /**
     * Returns true if this registry can't accept any more connections, else
     * false.
     */
    bool full() const { return connectionCount >= capacity; }

    /**
     * Returns the maximum number of connections that this registry can hold.
     */
    std::size_t maxSize() const { return capacity; }

private:
    static constexpr std::size_t DEFAULT_CAPACITY{100};

    /**
     * Returns a valid registry capacity for the given requested capacity.
     */
    static std::size_t getCapacity(std::size_t requestedCapacity)
    {
        if ((requestedCapacity == 0)
            || (requestedCapacity
                > std::numeric_limits<ConnectionHandle::ID>::max())) {
            LOG_INFO(
                "Invalid connection registry capacity: %zu. Defaulting to "
                "%zu.",
                requestedCapacity, DEFAULT_CAPACITY);
            return DEFAULT_CAPACITY;
        }
        return requestedCapacity;
    }

    struct Entry {
        ConnectionHandle::Generation generation{1};
        std::shared_ptr<ConnectionType> connection{};
    };

    /**
     * Returns the generation following the given generation, skipping 0.
     */
    static ConnectionHandle::Generation
        nextGeneration(ConnectionHandle::Generation generation)
    {
        if (generation
            == std::numeric_limits<ConnectionHandle::Generation>::max()) {
            return 1;
        }
        return static_cast<ConnectionHandle::Generation>(generation + 1);
    }

    std::size_t capacity;
    IDPool connectionIDPool;
    std::vector<Entry> entries;
    std::size_t connectionCount;
};

} // namespace AM
