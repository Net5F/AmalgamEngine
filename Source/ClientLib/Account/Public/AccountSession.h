#pragma once

#include "AccountConnectionEvent.h"
#include "AccountDefs.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "RegisterResponse.h"
#include "ServiceTicketIssued.h"
#include "QueuedEvents.h"
#include "entt/signal/sigh.hpp"
#include <array>
#include <string>

namespace AM
{
namespace Client
{
class Network;

/**
 * Owns the client's authenticated account state and controls account requests.
 *
 * Account credentials are forwarded directly to the AccountServer and are not
 * retained. A successful login's session token remains here for the lifetime
 * of the session so later service-ticket and logout requests can use it.
 *
 * All public functions, including tick(), must be called from the main thread.
 */
class AccountSession
{
public:
    enum class LoginState { LoggedOut, LoggingIn, LoggedIn, LoggingOut };

    AccountSession(Network& inNetwork,
                   EventDispatcher& networkEventDispatcher);
    ~AccountSession();

    AccountSession(const AccountSession&) = delete;
    AccountSession& operator=(const AccountSession&) = delete;

    /** Processes responses received from the AccountServer. */
    void tick();

    /** Starts a login request. Returns false if another request conflicts. */
    bool login(const std::string& username, const std::string& password);

    /** Starts a registration request. */
    bool registerAccount(const std::string& username,
                         const std::string& password);

    /** Revokes the current account session. */
    bool logout();

    /** Requests a one-use ticket for a WorldServer instance. */
    bool requestWorldTicket(Sint64 targetServerID);

    LoginState getLoginState() const noexcept;
    bool isAuthenticated() const noexcept;
    Sint64 getAccountID() const noexcept;
    Sint64 getIdleExpiresAt() const noexcept;
    Sint64 getAbsoluteExpiresAt() const noexcept;

private:
    void handleLoginResponse(LoginResponse& response);
    void handleRegisterResponse(RegisterResponse& response);
    void handleLogoutResponse(const LogoutResponse& response);
    void handleServiceTicketResponse(ServiceTicketIssued& response);
    void handleConnectionEvent(const AccountConnectionEvent& event);

    void clearSession() noexcept;

    Network& network;

    EventQueue<LoginResponse> loginResponseQueue;
    EventQueue<RegisterResponse> registerResponseQueue;
    EventQueue<LogoutResponse> logoutResponseQueue;
    EventQueue<ServiceTicketIssued> serviceTicketQueue;
    EventQueue<AccountConnectionEvent> connectionEventQueue;

    /** The current state of our various operations. */
    LoginState loginState;
    bool registrationPending;
    bool worldTicketRequestPending;

    /** If we're logged in, these are our current session's info. */
    Sint64 accountID;
    std::array<Uint8, SESSION_TOKEN_BYTES> sessionToken;
    Sint64 idleExpiresAt;
    Sint64 absoluteExpiresAt;

    entt::sigh<void(LoginResponse::Result)> loginCompletedSig;
    entt::sigh<void(const RegisterResponse&)> registrationCompletedSig;
    entt::sigh<void(LogoutResponse::Result)> logoutCompletedSig;
    entt::sigh<void(const ServiceTicketIssued&)>
        serviceTicketRequestCompletedSig;
    entt::sigh<void()> requestConnectionFailedSig;

public:
    /** A login request completed and the session model has been updated. */
    entt::sink<entt::sigh<void(LoginResponse::Result)>> loginCompleted;

    /** An account-registration request completed. */
    entt::sink<entt::sigh<void(const RegisterResponse&)>>
        registrationCompleted;

    /** A logout request completed and the session model has been updated. */
    entt::sink<entt::sigh<void(LogoutResponse::Result)>> logoutCompleted;

    /** A service-ticket request completed. */
    entt::sink<entt::sigh<void(const ServiceTicketIssued&)>>
        serviceTicketRequestCompleted;

    /** An in-progress request lost its AccountServer connection. */
    entt::sink<entt::sigh<void()>> requestConnectionFailed;
};

} // namespace Client
} // namespace AM
