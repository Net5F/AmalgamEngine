#include "AccountSession.h"
#include "Network.h"
#include "LoginRequest.h"
#include "LogoutRequest.h"
#include "RegisterRequest.h"
#include "RequestWorldTicket.h"
#include "Log.h"
#include <openssl/crypto.h>

namespace AM
{
namespace Client
{

AccountSession::AccountSession(Network& inNetwork,
                               EventDispatcher& networkEventDispatcher)
: network{inNetwork}
, loginResponseQueue{networkEventDispatcher}
, registerResponseQueue{networkEventDispatcher}
, logoutResponseQueue{networkEventDispatcher}
, serviceTicketQueue{networkEventDispatcher}
, connectionEventQueue{networkEventDispatcher}
, loginState{LoginState::LoggedOut}
, registrationPending{false}
, worldTicketRequestPending{false}
, accountID{0}
, sessionToken{}
, idleExpiresAt{0}
, absoluteExpiresAt{0}
, loginCompletedSig{}
, registrationCompletedSig{}
, logoutCompletedSig{}
, serviceTicketRequestCompletedSig{}
, requestConnectionFailedSig{}
, loginCompleted{loginCompletedSig}
, registrationCompleted{registrationCompletedSig}
, logoutCompleted{logoutCompletedSig}
, serviceTicketRequestCompleted{serviceTicketRequestCompletedSig}
, requestConnectionFailed{requestConnectionFailedSig}
{
}

AccountSession::~AccountSession()
{
    clearSession();
}

void AccountSession::tick()
{
    LoginResponse loginResponse{};
    while (loginResponseQueue.pop(loginResponse)) {
        handleLoginResponse(loginResponse);
    }

    RegisterResponse newRegisterResponse{};
    while (registerResponseQueue.pop(newRegisterResponse)) {
        handleRegisterResponse(newRegisterResponse);
    }

    LogoutResponse logoutResponse{};
    while (logoutResponseQueue.pop(logoutResponse)) {
        handleLogoutResponse(logoutResponse);
    }

    ServiceTicketIssued newServiceTicketResponse{};
    while (serviceTicketQueue.pop(newServiceTicketResponse)) {
        handleServiceTicketResponse(newServiceTicketResponse);
    }

    AccountConnectionEvent connectionEvent{};
    while (connectionEventQueue.pop(connectionEvent)) {
        handleConnectionEvent(connectionEvent);
    }
}

bool AccountSession::login(const std::string& username,
                           const std::string& password)
{
    if ((loginState != LoginState::LoggedOut) || registrationPending) {
        return false;
    }

    loginState = LoginState::LoggingIn;
    network.accountEndpoint.send(LoginRequest{username, password});
    return true;
}

bool AccountSession::registerAccount(const std::string& username,
                                     const std::string& password)
{
    if ((loginState != LoginState::LoggedOut) || registrationPending) {
        return false;
    }

    registrationPending = true;
    network.accountEndpoint.send(RegisterRequest{username, password});
    return true;
}

bool AccountSession::logout()
{
    if (loginState != LoginState::LoggedIn) {
        return false;
    }

    loginState = LoginState::LoggingOut;
    network.accountEndpoint.send(LogoutRequest{sessionToken});
    return true;
}

bool AccountSession::requestWorldTicket()
{
    if ((loginState != LoginState::LoggedIn) || worldTicketRequestPending) {
        return false;
    }

    worldTicketRequestPending = true;
    network.accountEndpoint.send(RequestWorldTicket{sessionToken});
    return true;
}

AccountSession::LoginState AccountSession::getLoginState() const noexcept
{
    return loginState;
}

bool AccountSession::isAuthenticated() const noexcept
{
    return (loginState == LoginState::LoggedIn)
           || (loginState == LoginState::LoggingOut);
}

Sint64 AccountSession::getAccountID() const noexcept
{
    return accountID;
}

Sint64 AccountSession::getIdleExpiresAt() const noexcept
{
    return idleExpiresAt;
}

Sint64 AccountSession::getAbsoluteExpiresAt() const noexcept
{
    return absoluteExpiresAt;
}

void AccountSession::handleLoginResponse(LoginResponse& response)
{
    if (loginState != LoginState::LoggingIn) {
        OPENSSL_cleanse(response.sessionToken.data(),
                        response.sessionToken.size());
        LOG_INFO("Received an unexpected login response.");
        return;
    }

    LoginResponse::Result result{response.result};
    if (response.result == LoginResponse::Success) {
        accountID = response.accountID;
        sessionToken = response.sessionToken;
        idleExpiresAt = response.idleExpiresAt;
        absoluteExpiresAt = response.absoluteExpiresAt;
        loginState = LoginState::LoggedIn;
    }
    else {
        clearSession();
    }

    OPENSSL_cleanse(response.sessionToken.data(), response.sessionToken.size());
    loginCompletedSig.publish(result);
}

void AccountSession::handleRegisterResponse(RegisterResponse& response)
{
    if (!registrationPending) {
        LOG_INFO("Received an unexpected registration response.");
        return;
    }

    registrationPending = false;
    registrationCompletedSig.publish(response);
    OPENSSL_cleanse(response.recoveryKey.data(), response.recoveryKey.size());
}

void AccountSession::handleLogoutResponse(const LogoutResponse& response)
{
    if (loginState != LoginState::LoggingOut) {
        LOG_INFO("Received an unexpected logout response.");
        return;
    }

    if ((response.result == LogoutResponse::Success)
        || (response.result == LogoutResponse::InvalidSession)) {
        clearSession();
    }
    else {
        loginState = LoginState::LoggedIn;
    }

    logoutCompletedSig.publish(response.result);
}

void AccountSession::handleServiceTicketResponse(ServiceTicketIssued& response)
{
    if (!worldTicketRequestPending) {
        OPENSSL_cleanse(response.ticket.data(), response.ticket.size());
        LOG_INFO("Received an unexpected service-ticket response.");
        return;
    }

    worldTicketRequestPending = false;
    if (response.result == ServiceTicketIssued::InvalidSession) {
        clearSession();
    }
    serviceTicketRequestCompletedSig.publish(response);
    OPENSSL_cleanse(response.ticket.data(), response.ticket.size());
}

void AccountSession::handleConnectionEvent(const AccountConnectionEvent& event)
{
    if (event.type == AccountConnectionEvent::Type::Connected) {
        return;
    }

    bool requestWasPending{(loginState == LoginState::LoggingIn)
                           || (loginState == LoginState::LoggingOut)
                           || registrationPending || worldTicketRequestPending};
    if (!requestWasPending) {
        // Account connections intentionally close after becoming idle. The
        // authenticated session remains valid independently of the socket.
        return;
    }

    registrationPending = false;
    worldTicketRequestPending = false;
    if (loginState == LoginState::LoggingIn) {
        clearSession();
    }
    else if (loginState == LoginState::LoggingOut) {
        loginState = LoginState::LoggedIn;
    }

    requestConnectionFailedSig.publish();
}

void AccountSession::clearSession() noexcept
{
    accountID = 0;
    OPENSSL_cleanse(sessionToken.data(), sessionToken.size());
    idleExpiresAt = 0;
    absoluteExpiresAt = 0;
    worldTicketRequestPending = false;
    loginState = LoginState::LoggedOut;
}

} // namespace Client
} // namespace AM
