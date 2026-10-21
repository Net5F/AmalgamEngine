#pragma once

#include "WorldClientEndpoint.h"
#include "WorldSimulation.h"
#include "PeriodicCaller.h"
#include "AccountClientEndpoint.h"
#include "AccountDefs.h"
#include "LoginResponse.h"
#include "ServiceTicketResponse.h"
#include "QueuedEvents.h"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <array>
#include <atomic>

namespace AM
{
namespace LTC
{
/**
 * Represents a single simulated client.
 * Maintains only as much state as is necessary to keep the connection going
 * and send inputs.
 *
 * Each client logs in as the load test account matching its index (see
 * LoadTestAccounts.h). The accounts must first be created using
 * SeedLoadTestAccounts.
 */
class SimulatedClient
{
public:
    /**
     * @param inAccountIndex The index of the load test account to log in as.
     * @param inInputsPerSecond How many inputs to send per second.
     * @param ioContext Used by our connections. Must be run by the caller.
     * @param accountSSLContext Used by our AccountServer connection. Must
     *                          verify AccountServer's certificate.
     * @param worldSSLContext Used by our WorldServer connection. Must verify
     *                        WorldServer's certificate.
     */
    SimulatedClient(unsigned int inAccountIndex, unsigned int inInputsPerSecond,
                    asio::io_context& ioContext,
                    asio::ssl::context& accountSSLContext,
                    asio::ssl::context& worldSSLContext);

    /**
     * Logs in to AccountServer, then uses the resulting ticket to connect
     * to WorldServer.
     *
     * Blocks until connected. Errors if any step fails.
     */
    void connect();

    /**
     * If we're connected, calls the sim and network ticks.
     */
    void tick();

private:
    /** How long to wait for an AccountServer response, in microseconds.
        Note: Logins are deliberately slow to process. */
    static constexpr std::int64_t ACCOUNT_RESPONSE_WAIT_US{10 * 1000 * 1000};

    /**
     * Logs in to AccountServer and requests a World Server ticket.
     *
     * Blocks until complete. Errors if any step fails.
     */
    std::array<Uint8, SERVICE_TICKET_BYTES> requestWorldTicket();

    /** The index of the load test account that we log in as. */
    const unsigned int accountIndex;

    /** Used to dispatch events from the network to the simulation. */
    EventDispatcher networkEventDispatcher;

    Client::WorldClientEndpoint worldEndpoint;
    PeriodicCaller networkCaller;

    WorldSimulation worldSim;
    PeriodicCaller simCaller;

    /** Used to log in and request World Server tickets. */
    Client::AccountClientEndpoint accountEndpoint;

    /** AccountServer responses, received from accountEndpoint. */
    EventQueue<LoginResponse> loginResponseQueue;
    EventQueue<ServiceTicketResponse> serviceTicketQueue;

    /** If true, this client is connected to the server and we've processed the
        ConnectionResponse. */
    std::atomic<bool> isConnected;
};

} // End namespace LTC
} // End namespace AM
