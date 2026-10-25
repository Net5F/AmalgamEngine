#include "SimulatedClient.h"
#include "LoadTestAccounts.h"
#include "MessageProcessorContext.h"
#include "LoginRequest.h"
#include "ServiceTicketRequest.h"
#include "SharedConfig.h"
#include "Timer.h"
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
                                 asio::ssl::context& accountSSLContext,
                                 asio::ssl::context& worldSSLContext)
: accountIndex{inAccountIndex}
, networkEventDispatcher{}
, worldEndpoint{ioContext, worldSSLContext,
                Client::MessageProcessorContext{networkEventDispatcher}}
, networkCaller(
      std::bind_front(&Client::WorldClientEndpoint::tick, &worldEndpoint),
      SharedConfig::CLIENT_NETWORK_TICK_TIMESTEP_S, "Network", true)
, worldSim(worldEndpoint, networkEventDispatcher, inInputsPerSecond)
, simCaller(std::bind_front(&WorldSimulation::tick, &worldSim),
            SharedConfig::SIM_TICK_TIMESTEP_S, "Sim", false)
, accountEndpoint{ioContext, accountSSLContext,
                  Client::MessageProcessorContext{networkEventDispatcher}}
, loginResponseQueue{networkEventDispatcher}
, serviceTicketQueue{networkEventDispatcher}
, isConnected(false)
{
    // Network stats are global, so only the first client logs them (they
    // cover every client's traffic).
    worldEndpoint.setNetstatsLoggingEnabled(accountIndex == 0);
}

void SimulatedClient::connect()
{
    // Get a ticket from AccountServer.
    std::array<Uint8, SERVICE_TICKET_BYTES> worldTicket{requestWorldTicket()};

    // Connect to WorldServer.
    worldSim.connect(worldTicket);

    // Start the tick timers at the current time, in phase with each other.
    double startTime{Timer::getGlobalTime()};
    simCaller.initTimer(startTime);
    networkCaller.initTimer(startTime);

    isConnected = true;
}

void SimulatedClient::tick()
{
    // Note: connect() runs on another thread. Once it sets isConnected, it's
    //       done touching our state.
    if (isConnected) {
        networkCaller.update();
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
    accountEndpoint.send(ServiceTicketRequest{
        ServiceTicketAudience::WorldServer, loginResponse.sessionToken});

    ServiceTicketResponse ticketResponse{};
    if (!(serviceTicketQueue.waitPop(ticketResponse,
                                     ACCOUNT_RESPONSE_WAIT_US))) {
        LOG_FATAL("AccountServer did not respond to ticket request. "
                  "Username: %s",
                  username.c_str());
    }
    if (ticketResponse.result != ServiceTicketResponse::Success) {
        LOG_FATAL("Failed to get World Server ticket for %s. Result: %u",
                  username.c_str(),
                  static_cast<unsigned int>(ticketResponse.result));
    }

    return ticketResponse.ticket;
}

} // End namespace LTC
} // End namespace AM
