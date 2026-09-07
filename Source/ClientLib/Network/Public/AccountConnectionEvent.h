#pragma once

namespace AM
{
namespace Client
{

/** Communicates a change in the AccountServer connection lifecycle. */
struct AccountConnectionEvent {
    enum class Type { Connected, ConnectionFailed, Disconnected };

    Type type{Type::Disconnected};
};

} // namespace Client
} // namespace AM
