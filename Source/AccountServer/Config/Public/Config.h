#pragma once

namespace AM
{
namespace AccountServer
{
/**
 * Module-specific configuration data.
 */
class Config
{
public:
    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    /** The port that the server listens for incoming client connections on. */
    static constexpr unsigned int SERVER_PORT{41498};

    /** The maximum number of clients that we will allow. */
    static constexpr unsigned int MAX_CLIENTS{1000};

    /** How long we should wait before considering the client to be timed out.
        Arbitrarily chosen. If too high, we set ourselves up to take a huge
        spike of data for a very late client. */
    static constexpr double CLIENT_TIMEOUT_S{4};
};

} // End namespace AccountServer
} // End namespace AM
