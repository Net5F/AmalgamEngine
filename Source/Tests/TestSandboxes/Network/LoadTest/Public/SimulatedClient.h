#pragma once

#include "NetworkSimulation.h"
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
     * @param ioContext Used by our AccountServer connection. Must be run by
     *                  the caller.
     * @param sslContext Used by our AccountServer connection. Must verify the
     *                   AccountServer's certificate.
     */
    SimulatedClient(unsigned int inAccountIndex, unsigned int inInputsPerSecond,
                    asio::io_context& ioContext,
                    asio::ssl::context& sslContext);

    /**
     * Logs in to the AccountServer, then uses the resulting ticket to connect
     * to the WorldServer.
     *
     * Blocks until connected. Errors if any step fails.
     */
    void connect();

    /**
     * Calls networkSim.receiveAndProcess().
     */
    void receiveAndProcess();

    /**
     * Calls the sim and network ticks.
     */
    void tick();

private:
    /** How long to wait for an AccountServer response, in microseconds.
        Note: Logins are deliberately slow to process. */
    static constexpr std::int64_t ACCOUNT_RESPONSE_WAIT_US{10 * 1000 * 1000};

    /**
     * Logs in to the AccountServer and requests a World Server ticket.
     *
     * Blocks until complete. Errors if any step fails.
     */
    std::array<Uint8, SERVICE_TICKET_BYTES> requestWorldTicket();

    /** The index of the load test account that we log in as. */
    const unsigned int accountIndex;

    NetworkSimulation networkSim;
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
