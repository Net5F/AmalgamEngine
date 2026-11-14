#pragma once

#include "ByteTools.h"
#include <type_traits>

namespace AM
{
/**
 * Holds a fixed-size secret value, such as a session token or service ticket,
 * and wipes it from memory when it's no longer needed.
 *
 * The value is wiped when this object is destroyed, and when it's moved from.
 * Copies are allowed (e.g. so the secret can be passed through event queues),
 * and each copy wipes itself in the same way.
 *
 * For text secrets, such as passwords, see SecretString.
 *
 * Note: T must be trivially copyable, so that it has no heap storage that
 *       could leave copies behind. Typically, T is a std::array of bytes.
 *
 * Note: To serialize, use the underlying value, e.g.
 *       "serializer.container1b(message.ticket.get());"
 */
template<typename T>
class Secret
{
public:
    static_assert(std::is_trivially_copyable_v<T>,
                  "Secret requires a trivially copyable type.");

    Secret() noexcept
    : value{}
    {
    }

    explicit Secret(const T& inValue) noexcept
    : value{inValue}
    {
    }

    Secret(const Secret& other) noexcept = default;

    Secret(Secret&& other) noexcept
    : value{other.value}
    {
        other.wipe();
    }

    Secret& operator=(const Secret& other) noexcept = default;

    Secret& operator=(Secret&& other) noexcept
    {
        if (this != &other) {
            value = other.value;
            other.wipe();
        }
        return *this;
    }

    ~Secret() { wipe(); }

    T& get() noexcept { return value; }
    const T& get() const noexcept { return value; }

    /**
     * Overwrites the value with zeros.
     */
    void wipe() noexcept { ByteTools::wipe(&value, sizeof(T)); }

private:
    T value;
};

} // End namespace AM
