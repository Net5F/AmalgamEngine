#pragma once

namespace AM
{
namespace Client
{

/**
 * Communicates that a connection error occurred.
 */
struct ConnectionError {
    enum class Type {
        Failed,    /*!< We failed to connect. */
        Disconnected /*!< We lost our connection to the server. */
    };

    /** The type of connection error that occurred. */
    Type type{Type::Disconnected};
};

} // End namespace Client
} // End namespace AM
