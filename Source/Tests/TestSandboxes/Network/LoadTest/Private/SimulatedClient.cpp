#include "SimulatedClient.h"
#include "LoadTestAccounts.h"
#include "MessageProcessorContext.h"
#include "LoginRequest.h"
#include "RequestWorldTicket.h"
#include "SharedConfig.h"
#include "Log.h"
#include <functional>
#include <string>

namespace AM
{
namespace LTC
{
SimulatedClient::SimulatedClient(unsigned int inAccountIndex,
                                 unsigned int inInputsPerSecond,
                                 asio::io_context& ioContext,
                                 asio::ssl::context& sslContext)
: accountIndex{inAccountIndex}
, networkSim{}
, networkCaller(std::bind_front(&NetworkSimulation::tick, &networkSim),
                SharedConfig::CLIENT_NETWORK_TICK_TIMESTEP_S, "Network", true)
, worldSim(networkSim, inInputsPerSecond)
, simCaller(std::bind_front(&WorldSimulation::tick, &worldSim),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, accountEndpoint{ioContext, sslContext,
                  Client::MessageProcessorContext{
                      networkSim.getEventDispatcher()}}
, loginResponseQueue{networkSim.getEventDispatcher()}
, serviceTicketQueue{networkSim.getEventDispatcher()}
, isConnected(false)
{
}

void SimulatedClient::connect()
{
    // Get a ticket from the AccountServer.
    std::array<Uint8, SERVICE_TICKET_BYTES> worldTicket{requestWorldTicket()};

    // Connect to the WorldServer.
    worldSim.connect(worldTicket);

    // Start the tick timer at the current time.
    simCaller.initTimer();
    networkCaller.initTimer();

    isConnected = true;
}

void SimulatedClient::receiveAndProcess()
{
    // Note: This is safe to call, even if connect() is running on another
    //       thread (it has an internal check).
    networkSim.receiveAndProcess();
}

void SimulatedClient::tick()
{
    // Process the network.
    networkCaller.update();

    // If we're connected, process the world sim.
    if (isConnected) {
        simCaller.update();
    }
}

std::array<Uint8, SERVICE_TICKET_BYTES> SimulatedClient::requestWorldTicket()
{
    // Log in.
    std::string username{getLoadTestUsername(accountIndex)};
    accountEndpoint.send(
        LoginRequest{username, std::string{LOAD_TEST_PASSWORD}});

    LoginResponse loginResponse{};
    if (!(loginResponseQueue.waitPop(loginResponse,
                                     ACCOUNT_RESPONSE_WAIT_US))) {
        LOG_FATAL("AccountServer did not respond to login. Username: %s",
                  username.c_str());
    }
    if (loginResponse.result == LoginResponse::InvalidAccountDetails) {
        LOG_FATAL("Invalid account details for %s. Did you run "
                  "SeedLoadTestAccounts?",
                  username.c_str());
    }
    else if (loginResponse.result != LoginResponse::Success) {
        LOG_FATAL("Failed to log in as %s. Result: %u", username.c_str(),
                  static_cast<unsigned int>(loginResponse.result));
    }

    // Request a World Server ticket.
    accountEndpoint.send(RequestWorldTicket{loginResponse.sessionToken});

    ServiceTicketIssued ticketResponse{};
    if (!(serviceTicketQueue.waitPop(ticketResponse,
                                     ACCOUNT_RESPONSE_WAIT_US))) {
        LOG_FATAL("AccountServer did not respond to ticket request. "
                  "Username: %s",
                  username.c_str());
    }
    if (ticketResponse.result != ServiceTicketIssued::Success) {
        LOG_FATAL("Failed to get World Server ticket for %s. Result: %u",
                  username.c_str(),
                  static_cast<unsigned int>(ticketResponse.result));
    }

    return ticketResponse.ticket;
}

} // End namespace LTC
} // End namespace AM
