#pragma once

#include "ConnectionHandle.h"
#include "IDPool.h"
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
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
    : capacity{inCapacity}
    , networkIDPool{IDPool::ReservationStrategy::MarchForward,
                    getPoolSize(inCapacity)}
    , entries{}
    , nextGeneration{1}
    {
        networkIDPool.markIDAsReserved(NULL_NETWORK_ID);

        entries.reserve(capacity);
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

        NetworkID networkID{static_cast<NetworkID>(networkIDPool.reserveID())};
        ConnectionHandle handle{networkID, reserveGeneration()};
        bool inserted{entries
                          .try_emplace(networkID, Entry{handle.generation,
                                                        std::move(connection)})
                          .second};
        if (!inserted) {
            networkIDPool.freeID(networkID);
            return std::nullopt;
        }

        return handle;
    }

    /**
     * If the given handle matches a valid connection, returns it. Otherwise
     * returns nullptr.
     */
    std::shared_ptr<ConnectionType> find(ConnectionHandle handle) const
    {
        auto iterator{entries.find(handle.networkID)};
        if ((iterator == entries.end())
            || (iterator->second.generation != handle.generation)) {
            return {};
        }
        return iterator->second.connection;
    }

    /**
     * Attempts to delete the given connection from this registry.
     * @return true if erased, else false (connection not found).
     */
    bool erase(ConnectionHandle handle)
    {
        auto iterator{entries.find(handle.networkID)};
        if ((iterator == entries.end())
            || (iterator->second.generation != handle.generation)) {
            return false;
        }

        std::shared_ptr<ConnectionType> connection{
            std::move(iterator->second.connection)};
        entries.erase(iterator);
        networkIDPool.freeID(handle.networkID);
        connection->close();
        return true;
    }

    /**
     * Closes all connections in this registry.
     */
    void closeAll()
    {
        std::vector<std::shared_ptr<ConnectionType>> connections{};
        connections.reserve(entries.size());
        for (auto& [networkID, entry] : entries) {
            networkIDPool.freeID(networkID);
            connections.push_back(std::move(entry.connection));
        }
        entries.clear();

        for (const std::shared_ptr<ConnectionType>& connection : connections) {
            connection->close();
        }
    }

    /**
     * Returns the number of connections in this registry.
     */
    std::size_t size() const { return entries.size(); }

    /**
     * Returns true if this registry is empty, else false.
     */
    bool empty() const { return entries.empty(); }

    /**
     * Returns true if this registry can't accept any more connections, else
     * false.
     */
    bool full() const { return entries.size() >= capacity; }

    /**
     * Returns the maximum number of connections that this registry can hold.
     */
    std::size_t maxSize() const { return capacity; }

private:
    /**
     * Returns an appropriate size of IDPool for the given requested capacity.
     */
    static std::size_t getPoolSize(std::size_t requestedCapacity)
    {
        if ((requestedCapacity == 0)
            || (requestedCapacity > std::numeric_limits<NetworkID>::max())) {
            LOG_INFO(
                "Invalid connection registry capacity: %zu. Defaulting to 100.",
                requestedCapacity.);
            return 100 + 1;
        }
        return requestedCapacity + 1;
    }

    struct Entry {
        Uint64 generation;
        std::shared_ptr<ConnectionType> connection;
    };

    /**
     * Returns the next generation to use for a connection handle.
     */
    Uint64 reserveGeneration()
    {
        Uint64 generation{nextGeneration++};
        if (nextGeneration == 0) {
            nextGeneration = 1;
        }
        return generation;
    }

    std::size_t capacity;
    IDPool networkIDPool;
    std::unordered_map<NetworkID, Entry> entries;
    Uint64 nextGeneration;
};

} // namespace AM
