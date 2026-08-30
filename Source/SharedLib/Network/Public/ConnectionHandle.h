#pragma once

#include <SDL3/SDL_stdinc.h>
#include <limits>

namespace AM
{
template<typename ConnectionType>
class ConnectionRegistry;

/**
 * Stable local reference to a registered connection.
 *
 * The generation prevents delayed asynchronous work from addressing a newer
 * connection that happens to reuse the same ID. Handles are local to their
 * owning registry and must not be serialized.
 */
class ConnectionHandle
{
public:
    using Value = Uint32;
    using ID = Uint16;
    using Generation = Uint16;

    constexpr ConnectionHandle() = default;

    explicit constexpr operator bool() const
    {
        return (id() != 0) && (generation() != 0);
    }

    constexpr bool operator==(const ConnectionHandle&) const = default;

private:
    template<typename ConnectionType>
    friend class ConnectionRegistry;

    static constexpr unsigned int ID_BITS{std::numeric_limits<ID>::digits};
    static constexpr Value ID_MASK{std::numeric_limits<ID>::max()};

    static_assert(std::numeric_limits<Value>::digits
                  == (std::numeric_limits<ID>::digits
                      + std::numeric_limits<Generation>::digits));

    constexpr ConnectionHandle(ID inID, Generation inGeneration)
    : packedValue{(static_cast<Value>(inGeneration) << ID_BITS)
                  | static_cast<Value>(inID)}
    {
    }

    constexpr ID id() const
    {
        return static_cast<ID>(packedValue & ID_MASK);
    }

    constexpr Generation generation() const
    {
        return static_cast<Generation>(packedValue >> ID_BITS);
    }

    Value packedValue{0};
};

static_assert(sizeof(ConnectionHandle) == sizeof(ConnectionHandle::Value));

inline constexpr ConnectionHandle NULL_CONNECTION_HANDLE{};

} // namespace AM
