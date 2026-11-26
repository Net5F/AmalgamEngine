#pragma once

#include "AccountConnectionEvent.h"
#include "AccountDefs.h"
#include "LoginResponse.h"
#include "LogoutResponse.h"
#include "RecoverAccountResponse.h"
#include "RegisterResponse.h"
#include "ServiceTicketResponse.h"
#include "BanResponse.h"
#include "UnbanResponse.h"
#include "SetRoleResponse.h"
#include "QueuedEvents.h"
#include "entt/signal/sigh.hpp"
#include <array>
#include <string>
#include <string_view>
#include <vector>

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

    /**
     * Requests to ban the given account. Returns false if we aren't logged
     * in.
     *
     * @param durationS How long the ban lasts. 0 means permanent. Must be at
     *                  most BanRequest::MAX_DURATION_S.
     * @param reason (Optional) Why the ban was given.
     */
    bool requestBan(const std::string& targetUsername, Sint64 durationS,
                    const std::string& reason);

    /**
     * Requests to lift the given account's active ban. Returns false if we
     * aren't logged in.
     *
     * @param reason (Optional) Why the ban was lifted.
     */
    bool requestUnban(const std::string& targetUsername,
                      const std::string& reason);

    /**
     * Requests to change the given account's role. Returns false if we aren't
     * logged in.
     *
     * @param reason (Optional) Why the role was changed.
     */
    bool requestSetRole(const std::string& targetUsername,
                        const std::string& roleID, const std::string& reason);

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

    /**
     * Fills in the given request's session token, sends it, and tracks it as
     * pending.
     *
     * Used for requests that may be pending more than once at a time (e.g.
     * BanRequest).
     *
     * @param failedResponse The response to publish if the connection drops
     *                       before AccountServer responds.
     */
    template<typename Request, typename Response>
    bool sendTrackedRequest(Request& request, Response failedResponse,
                            std::vector<Response>& pendingRequests);

    /**
     * Removes one of the given pending requests, and publishes the given
     * response.
     */
    template<typename Response>
    void handleTrackedResponse(const Response& response,
                               std::vector<Response>& pendingRequests,
                               entt::sigh<void(const Response&)>& completedSig);

    /**
     * Publishes the failed response of each of the given pending requests,
     * and clears them.
     */
    template<typename Response>
    void failPendingRequests(std::vector<Response>& pendingRequests,
                             entt::sigh<void(const Response&)>& completedSig);

    Network& network;

    EventQueue<LoginResponse> loginResponseQueue;
    EventQueue<RegisterResponse> registerResponseQueue;
    EventQueue<LogoutResponse> logoutResponseQueue;
    EventQueue<RecoverAccountResponse> recoverAccountResponseQueue;
    EventQueue<ServiceTicketResponse> serviceTicketQueue;
    EventQueue<BanResponse> banResponseQueue;
    EventQueue<UnbanResponse> unbanResponseQueue;
    EventQueue<SetRoleResponse> setRoleResponseQueue;
    EventQueue<AccountConnectionEvent> connectionEventQueue;

    /** The current state of our various operations. */
    LoginState loginState;
    bool registrationPending;
    bool recoveryPending;
    /** Tracks which audiences have a service ticket request pending. */
    std::array<bool, static_cast<std::size_t>(ServiceTicketAudience::Count)>
        serviceTicketRequestPending;
    /** The ban, unban, and set role requests that are awaiting a response,
        stored as the responses to publish if the connection drops. */
    std::vector<BanResponse> pendingBanRequests;
    std::vector<UnbanResponse> pendingUnbanRequests;
    std::vector<SetRoleResponse> pendingSetRoleRequests;

    /** If we're logged in, these are our current session's info. */
    Sint64 accountID;
    SessionToken sessionToken;
    Sint64 idleExpiresAt;
    Sint64 absoluteExpiresAt;

    entt::sigh<void(const LoginResponse&)> loginCompletedSig;
    entt::sigh<void(const RegisterResponse&)> registrationCompletedSig;
    entt::sigh<void(LogoutResponse::Result)> logoutCompletedSig;
    entt::sigh<void(const RecoverAccountResponse&)> recoveryCompletedSig;
    entt::sigh<void(const ServiceTicketResponse&)>
        serviceTicketRequestCompletedSig;
    entt::sigh<void(const BanResponse&)> banRequestCompletedSig;
    entt::sigh<void(const UnbanResponse&)> unbanRequestCompletedSig;
    entt::sigh<void(const SetRoleResponse&)> setRoleRequestCompletedSig;
    entt::sigh<void()> requestConnectionFailedSig;

public:
    /** A login request completed and the session model has been updated. */
    entt::sink<entt::sigh<void(const LoginResponse&)>> loginCompleted;

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

    /** A ban, unban, or set role request completed.
        If the AccountServer connection is lost while a request is pending,
        these are published with result == InternalError. */
    entt::sink<entt::sigh<void(const BanResponse&)>> banRequestCompleted;
    entt::sink<entt::sigh<void(const UnbanResponse&)>> unbanRequestCompleted;
    entt::sink<entt::sigh<void(const SetRoleResponse&)>>
        setRoleRequestCompleted;

    /** An in-progress login, logout, registration, or recovery request lost
        its AccountServer connection. */
    entt::sink<entt::sigh<void()>> requestConnectionFailed;
};

} // namespace Client
} // namespace AM
