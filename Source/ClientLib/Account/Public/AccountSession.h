#pragma once

#include "AccountConnectionEvent.h"
#include "AccountDefs.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "RecoverAccountResponse.h"
#include "RegisterResponse.h"
#include "ServiceTicketResponse.h"
#include "QueuedEvents.h"
#include "entt/signal/sigh.hpp"
#include <array>
#include <string>
#include <string_view>

namespace AM
{
namespace Client
{
class Network;

/**
 * Owns the client's authenticated account state and controls account requests.
 *
 * Account credentials are forwarded directly to AccountServer and are not
 * retained. A successful login's session token remains here for the lifetime
 * of the session so later service-ticket and logout requests can use it.
 *
 * All public functions, including tick(), must be called from the main thread.
 */
class AccountSession
{
public:
    enum class LoginState { LoggedOut, LoggingIn, LoggedIn, LoggingOut };

    AccountSession(Network& inNetwork, EventDispatcher& networkEventDispatcher);
    ~AccountSession();

    AccountSession(const AccountSession&) = delete;
    AccountSession& operator=(const AccountSession&) = delete;

    /** Processes responses received from AccountServer. */
    void tick();

    /** Starts a login request. Returns false if another request conflicts,
        or if the password is longer than LoginRequest::PASSWORD_MAX. */
    bool login(const std::string& username, std::string_view password);

    /** Starts a registration request. Returns false if another request
        conflicts, or if the password is longer than
        RegisterRequest::PASSWORD_MAX. */
    bool registerAccount(const std::string& username,
                         std::string_view password);

    /** Starts an account recovery request, which uses the account's recovery
        key to set a new password. Returns false if another request conflicts,
        or if the key or password is too long. */
    bool recoverAccount(const std::string& username,
                        std::string_view recoveryKey,
                        std::string_view newPassword);

    /** Revokes the current account session. */
    bool logout();

    /**
     * Requests a one-use ticket for the given service. Returns false if we
     * aren't logged in, or if a request for this audience is already pending.
     */
    bool requestServiceTicket(ServiceTicketAudience audience);

    LoginState getLoginState() const noexcept;
    bool isAuthenticated() const noexcept;
    Sint64 getAccountID() const noexcept;
    Sint64 getIdleExpiresAt() const noexcept;
    Sint64 getAbsoluteExpiresAt() const noexcept;

private:
    void handleLoginResponse(LoginResponse& response);
    void handleRegisterResponse(RegisterResponse& response);
    void handleLogoutResponse(const LogoutResponse& response);
    void handleRecoverAccountResponse(RecoverAccountResponse& response);
    void handleServiceTicketResponse(ServiceTicketResponse& response);
    void handleConnectionEvent(const AccountConnectionEvent& event);

    void clearSession() noexcept;

    Network& network;

    EventQueue<LoginResponse> loginResponseQueue;
    EventQueue<RegisterResponse> registerResponseQueue;
    EventQueue<LogoutResponse> logoutResponseQueue;
    EventQueue<RecoverAccountResponse> recoverAccountResponseQueue;
    EventQueue<ServiceTicketResponse> serviceTicketQueue;
    EventQueue<AccountConnectionEvent> connectionEventQueue;

    /** The current state of our various operations. */
    LoginState loginState;
    bool registrationPending;
    bool recoveryPending;
    /** Tracks which audiences have a service ticket request pending. */
    std::array<bool, static_cast<std::size_t>(ServiceTicketAudience::Count)>
        serviceTicketRequestPending;

    /** If we're logged in, these are our current session's info. */
    Sint64 accountID;
    SessionToken sessionToken;
    Sint64 idleExpiresAt;
    Sint64 absoluteExpiresAt;

    entt::sigh<void(LoginResponse::Result)> loginCompletedSig;
    entt::sigh<void(const RegisterResponse&)> registrationCompletedSig;
    entt::sigh<void(LogoutResponse::Result)> logoutCompletedSig;
    entt::sigh<void(const RecoverAccountResponse&)> recoveryCompletedSig;
    entt::sigh<void(const ServiceTicketResponse&)>
        serviceTicketRequestCompletedSig;
    entt::sigh<void()> requestConnectionFailedSig;

public:
    /** A login request completed and the session model has been updated. */
    entt::sink<entt::sigh<void(LoginResponse::Result)>> loginCompleted;

    /** An account-registration request completed. */
    entt::sink<entt::sigh<void(const RegisterResponse&)>> registrationCompleted;

    /** A logout request completed and the session model has been updated. */
    entt::sink<entt::sigh<void(LogoutResponse::Result)>> logoutCompleted;

    /** An account recovery request completed. */
    entt::sink<entt::sigh<void(const RecoverAccountResponse&)>>
        recoveryCompleted;

    /** A service-ticket request completed. Check the response's audience to
        see which request it was for.
        If the AccountServer connection is lost while a request is pending,
        this is published with result == InternalError. */
    entt::sink<entt::sigh<void(const ServiceTicketResponse&)>>
        serviceTicketRequestCompleted;

    /** An in-progress login, logout, registration, or recovery request lost
        its AccountServer connection. */
    entt::sink<entt::sigh<void()>> requestConnectionFailed;
};

} // namespace Client
} // namespace AM
