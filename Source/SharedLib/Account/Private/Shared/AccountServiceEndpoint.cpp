#include "Shared/AccountServiceEndpoint.h"
#include "ConsumeServiceTicketRequest.h"
#include "SaveAccountDataRequest.h"
#include "ServiceHeartbeat.h"
#include "Deserialize.h"
#include "Log.h"
#include "VariantTools.h"
#include "asio/error.hpp"
#include "asio/ip/address.hpp"
#include "asio/post.hpp"
#include "asio/system_error.hpp"
#include <algorithm>
#include <chrono>
#include <limits>
#include <utility>

namespace AM
{
namespace
{
/** Returns the given number of seconds as a timer duration. */
asio::steady_timer::duration toTimerDuration(double seconds)
{
    return std::chrono::duration_cast<asio::steady_timer::duration>(
        std::chrono::duration<double>(seconds));
}
} // namespace

AccountServiceEndpoint::AccountServiceEndpoint(asio::io_context& inIoContext,
                                               asio::ssl::context& inSSLContext,
                                               const std::string& inServerIP,
                                               unsigned int inServerPort)
: ioContext{inIoContext}
, sslContext{inSSLContext}
, serverIP{inServerIP}
, serverPort{inServerPort}
, connectionTimer{inIoContext}
, reconnectTimer{inIoContext}
, heartbeatTimer{inIoContext}
, connectingSocket{}
, connection{}
, messageFramer{MAX_WRITE_PAYLOAD_SIZE}
, pendingRequests{}
, nextRequestID{0}
, reconnectDelayS{RECONNECT_MIN_DELAY_S}
, started{false}
, connectionState{ConnectionState::Disconnected}
, connectionAttempt{0}
{
}

AccountServiceEndpoint::~AccountServiceEndpoint()
{
    // Note: We intentionally don't call any pending callbacks, since the
    //       objects that they reference may already be destroyed.
    cancelTimer(connectionTimer);
    cancelTimer(reconnectTimer);
    cancelTimer(heartbeatTimer);
    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
    }
    if (connection) {
        connection->close();
    }
}

void AccountServiceEndpoint::start()
{
    asio::post(ioContext, [this]() {
        if (started) {
            LOG_INFO(
                "Attempted to start AccountServiceEndpoint more than once.");
            return;
        }

        started = true;
        connectOnIOThread();
    });
}

void AccountServiceEndpoint::consumeServiceTicket(
    const ServiceTicket& ticket, bool includeAccountData,
    ConsumeServiceTicketCallback callback)
{
    asio::post(ioContext, [this, ticket, includeAccountData,
                           callback = std::move(callback)]() mutable {
        consumeServiceTicketOnIOThread(ticket, includeAccountData,
                                       std::move(callback));
    });
}

void AccountServiceEndpoint::saveAccountData(Sint64 accountID,
                                             AccountData accountData,
                                             SaveAccountDataCallback callback)
{
    asio::post(ioContext, [this, accountID,
                           accountData = std::move(accountData),
                           callback = std::move(callback)]() mutable {
        saveAccountDataOnIOThread(accountID, accountData, std::move(callback));
    });
}

void AccountServiceEndpoint::consumeServiceTicketOnIOThread(
    const ServiceTicket& ticket, bool includeAccountData,
    ConsumeServiceTicketCallback callback)
{
    if (!callback) {
        LOG_INFO("Ignoring a service ticket request with no callback.");
        return;
    }

    PendingCallback pendingCallback{std::move(callback)};
    std::optional<Uint32> requestID{beginRequest(pendingCallback)};
    if (!requestID) {
        return;
    }

    // Frame and send the request.
    BinaryBufferSharedPtr framedMessage{messageFramer.frameMessage(
        ConsumeServiceTicketRequest{.requestID{*requestID},
                                    .ticket{ticket},
                                    .includeAccountData{includeAccountData}})};
    sendRequest(*requestID, std::move(framedMessage));
}

void AccountServiceEndpoint::saveAccountDataOnIOThread(
    Sint64 accountID, AccountData& accountData,
    SaveAccountDataCallback callback)
{
    if (!callback) {
        LOG_INFO("Ignoring an account data save request with no callback.");
        return;
    }

    // If the data is too large to serialize, fail immediately.
    if (!(accountData.isWithinSizeLimits())) {
        LOG_ERROR("Failed to save account data: Data exceeds size limit. "
                  "AccountID: %lld",
                  static_cast<long long>(accountID));
        callback(SaveAccountDataResponse{
            .result{SaveAccountDataResponse::InternalError}});
        return;
    }

    PendingCallback pendingCallback{std::move(callback)};
    std::optional<Uint32> requestID{beginRequest(pendingCallback)};
    if (!requestID) {
        return;
    }

    // Frame and send the request.
    BinaryBufferSharedPtr framedMessage{messageFramer.frameMessage(
        SaveAccountDataRequest{.requestID{*requestID},
                               .accountID{accountID},
                               .accountData{std::move(accountData)}})};
    sendRequest(*requestID, std::move(framedMessage));
}

void AccountServiceEndpoint::connectOnIOThread()
{
    if (connectionState != ConnectionState::Disconnected) {
        return;
    }

    connectionState = ConnectionState::Connecting;
    beginConnect();
}

void AccountServiceEndpoint::beginConnect()
{
    if (connectionState != ConnectionState::Connecting) {
        return;
    }

    const std::size_t currentAttempt{++connectionAttempt};
    asio::error_code error{};
    asio::ip::address serverAddress{asio::ip::make_address(serverIP, error)};
    if (error || (serverPort > std::numeric_limits<Uint16>::max())) {
        if (!error) {
            error = asio::error::make_error_code(asio::error::invalid_argument);
        }
        handleConnectionFailure(currentAttempt, error);
        return;
    }

    connectingSocket = std::make_shared<asio::ip::tcp::socket>(ioContext);
    std::shared_ptr<asio::ip::tcp::socket> socket{connectingSocket};

    // Attempt to connect, with a timeout.
    try {
        connectionTimer.expires_after(toTimerDuration(CONNECTING_TIMEOUT_S));
        connectionTimer.async_wait(
            [this, socket, currentAttempt](const asio::error_code& timerError) {
                if (timerError || (currentAttempt != connectionAttempt)
                    || (connectionState != ConnectionState::Connecting)) {
                    return;
                }

                asio::error_code ignoredError{};
                socket->cancel(ignoredError);
                socket->close(ignoredError);
                handleConnectionFailure(
                    currentAttempt,
                    asio::error::make_error_code(asio::error::timed_out));
            });

        socket->async_connect(
            asio::ip::tcp::endpoint{serverAddress,
                                    static_cast<unsigned short>(serverPort)},
            [this, socket,
             currentAttempt](const asio::error_code& connectError) {
                onSocketConnected(socket, currentAttempt, connectError);
            });
    } catch (const asio::system_error& e) {
        handleConnectionFailure(currentAttempt, e.code());
    }
}

void AccountServiceEndpoint::handleConnectionFailure(
    std::size_t currentAttempt, const asio::error_code& error)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    // Tear down the connection.
    connectionState = ConnectionState::Disconnected;
    ++connectionAttempt;
    cancelTimer(connectionTimer);
    asio::error_code ignoredError{};
    if (connectingSocket) {
        connectingSocket->cancel(ignoredError);
        connectingSocket->close(ignoredError);
    }
    connectingSocket.reset();
    connection.reset();

    LOG_INFO("AccountServer connection failed: %s", error.message().c_str());
    failAllRequests();
    scheduleReconnect();
}

void AccountServiceEndpoint::disconnectWithError(const asio::error_code& error)
{
    // Note: We hold a local reference, since disconnecting will reset our
    //       member.
    if (std::shared_ptr<Connection> activeConnection{connection}) {
        activeConnection->disconnect(error);
    }
}

void AccountServiceEndpoint::scheduleReconnect()
{
    LOG_INFO("Reconnecting to AccountServer in %.0fs.", reconnectDelayS);

    try {
        reconnectTimer.expires_after(toTimerDuration(reconnectDelayS));
        reconnectTimer.async_wait([this](const asio::error_code& timerError) {
            if (timerError) {
                return;
            }
            connectOnIOThread();
        });
    } catch (const asio::system_error& e) {
        LOG_ERROR("Failed to schedule AccountServer reconnect: %s",
                  e.code().message().c_str());
    }

    reconnectDelayS = std::min(reconnectDelayS * 2, RECONNECT_MAX_DELAY_S);
}

void AccountServiceEndpoint::scheduleHeartbeat(std::size_t currentAttempt)
{
    try {
        heartbeatTimer.expires_after(toTimerDuration(HEARTBEAT_INTERVAL_S));
        heartbeatTimer.async_wait(
            [this, currentAttempt](const asio::error_code& timerError) {
                if (timerError || (currentAttempt != connectionAttempt)
                    || (connectionState != ConnectionState::Connected)) {
                    return;
                }

                // Note: We hold a local reference, since a failed send will
                //       disconnect and reset our member. In that case, we'll
                //       reconnect, so there's nothing else to do here.
                if (std::shared_ptr<Connection> activeConnection{connection}) {
                    activeConnection->send(ServiceHeartbeat{});
                }

                if ((currentAttempt == connectionAttempt)
                    && (connectionState == ConnectionState::Connected)) {
                    scheduleHeartbeat(currentAttempt);
                }
            });
    } catch (const asio::system_error& e) {
        disconnectWithError(e.code());
    }
}

void AccountServiceEndpoint::cancelTimer(asio::steady_timer& timer)
{
    try {
        timer.cancel();
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to cancel AccountServer timer: %s",
                 e.code().message().c_str());
    }
}

std::optional<Uint32>
    AccountServiceEndpoint::beginRequest(PendingCallback& callback)
{
    // If we aren't connected, fail immediately.
    if (connectionState != ConnectionState::Connected) {
        LOG_INFO("Failed to send an AccountServer request: Not connected.");
        failCallback(0, callback);
        return std::nullopt;
    }

    // If we already have too many requests in flight, fail immediately.
    if (pendingRequests.size() >= MAX_PENDING_REQUESTS) {
        LOG_INFO("Failed to queue an AccountServer request: Too many pending "
                 "requests.");
        failCallback(0, callback);
        return std::nullopt;
    }

    // Track the request so we can match it to its response.
    const Uint32 requestID{nextRequestID++};
    pendingRequests.emplace(requestID, std::move(callback));
    return requestID;
}

void AccountServiceEndpoint::sendRequest(Uint32 requestID,
                                         BinaryBufferSharedPtr framedMessage)
{
    // Note: We hold a local reference, since a failed send will disconnect
    //       and reset our member.
    std::shared_ptr<Connection> activeConnection{connection};
    if (!framedMessage || !activeConnection
        || !activeConnection->sendFramed(std::move(framedMessage))) {
        LOG_INFO("Failed to send an AccountServer request.");

        // Note: If the send failed because the write queue overflowed, the
        //       connection will have disconnected and already failed this
        //       request. In that case, this does nothing.
        failRequest(requestID);
    }
}

template<typename Response>
void AccountServiceEndpoint::completeRequest(Uint32 requestID,
                                             const Response& response)
{
    auto requestIt{pendingRequests.find(requestID)};
    if (requestIt == pendingRequests.end()) {
        LOG_INFO("Received a response for an unknown AccountServer request: "
                 "%u",
                 requestID);
        return;
    }

    // Note: We remove the request before calling the callback, in case the
    //       callback causes more requests to be completed.
    PendingCallback callback{std::move(requestIt->second)};
    pendingRequests.erase(requestIt);

    using Callback = std::function<void(const Response&)>;
    if (Callback * typedCallback{std::get_if<Callback>(&callback)}) {
        (*typedCallback)(response);
    }
    else {
        LOG_INFO("Received a mismatched response type for AccountServer "
                 "request: %u",
                 requestID);
        failCallback(requestID, callback);
    }
}

void AccountServiceEndpoint::failCallback(Uint32 requestID,
                                          PendingCallback& callback)
{
    std::visit(
        VariantTools::Overload{
            [&](ConsumeServiceTicketCallback& consumeCallback) {
                consumeCallback(ConsumeServiceTicketResponse{
                    .requestID{requestID},
                    .result{ConsumeServiceTicketResponse::InternalError}});
            },
            [&](SaveAccountDataCallback& saveCallback) {
                saveCallback(SaveAccountDataResponse{
                    .requestID{requestID},
                    .result{SaveAccountDataResponse::InternalError}});
            }},
        callback);
}

void AccountServiceEndpoint::failRequest(Uint32 requestID)
{
    auto requestIt{pendingRequests.find(requestID)};
    if (requestIt == pendingRequests.end()) {
        return;
    }

    // Note: We remove the request before calling the callback, in case the
    //       callback causes more requests to be completed.
    PendingCallback callback{std::move(requestIt->second)};
    pendingRequests.erase(requestIt);

    failCallback(requestID, callback);
}

void AccountServiceEndpoint::failAllRequests()
{
    // Note: We move the requests into a local first, in case a callback
    //       causes the map to be modified.
    std::unordered_map<Uint32, PendingCallback> failedRequests{
        std::move(pendingRequests)};
    pendingRequests.clear();

    for (auto& [requestID, callback] : failedRequests) {
        failCallback(requestID, callback);
    }
}

void AccountServiceEndpoint::onSocketConnected(
    std::shared_ptr<asio::ip::tcp::socket> socket, std::size_t currentAttempt,
    const asio::error_code& error)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    cancelTimer(connectionTimer);
    if (error) {
        handleConnectionFailure(currentAttempt, error);
        return;
    }

    // Socket successfully connected. Configure and create the new connection.
    TlsTransport::ClassConfig tlsTransportConfig{
        .handshakeTimeoutS{CONNECTING_TIMEOUT_S},
        .maxQueuedWriteBytes{MAX_QUEUED_WRITE_BYTES},
        .maxQueuedMessages{MAX_QUEUED_WRITES}};
    Connection::ClassConfig connectionConfig{
        .tlsTransportConfig{tlsTransportConfig},
        .partialReceiveTimeoutS{PARTIAL_RECEIVE_TIMEOUT_S},
        .idleTimeoutS{IDLE_TIMEOUT_S},
        .maxReadPayloadSize{MAX_READ_PAYLOAD_SIZE},
        .maxWritePayloadSize{MAX_WRITE_PAYLOAD_SIZE}};

    connection = std::make_shared<Connection>(std::move(*socket), sslContext,
                                              connectionConfig);
    connectingSocket.reset();

    // Register callbacks and start the handshake process.
    // Note: AccountServer's certificate is verified against its pin
    //       during the handshake, and it verifies ours.
    connection->setMessageCallback(
        [this](AccountServiceMessageType messageType,
               std::span<const Uint8> messageBuffer) {
            onMessageReceived(messageType, messageBuffer);
        });
    connection->startClient(
        [this, currentAttempt](AM::Connection&) {
            onConnectionReady(currentAttempt);
        },
        [this, currentAttempt](const asio::error_code& disconnectError) {
            onConnectionDisconnected(currentAttempt, disconnectError);
        });
}

void AccountServiceEndpoint::onConnectionReady(std::size_t currentAttempt)
{
    if ((currentAttempt != connectionAttempt)
        || (connectionState != ConnectionState::Connecting)) {
        return;
    }

    connectionState = ConnectionState::Connected;
    reconnectDelayS = RECONNECT_MIN_DELAY_S;
    LOG_INFO("Connected to AccountServer.");
    scheduleHeartbeat(currentAttempt);
}

void AccountServiceEndpoint::onConnectionDisconnected(
    std::size_t currentAttempt, const asio::error_code& error)
{
    if (currentAttempt != connectionAttempt) {
        return;
    }

    ConnectionState previousState{connectionState};
    connectionState = ConnectionState::Disconnected;
    ++connectionAttempt;
    cancelTimer(heartbeatTimer);
    connection.reset();
    connectingSocket.reset();

    if (previousState == ConnectionState::Connecting) {
        LOG_INFO("AccountServer connection failed: %s",
                 error.message().c_str());
    }
    else {
        LOG_INFO("AccountServer disconnected: %s", error.message().c_str());
    }

    failAllRequests();
    scheduleReconnect();
}

void AccountServiceEndpoint::onMessageReceived(
    AccountServiceMessageType messageType, std::span<const Uint8> messageBuffer)
{
    switch (messageType) {
        case AccountServiceMessageType::ConsumeServiceTicketResponse: {
            handleResponse<ConsumeServiceTicketResponse>(messageBuffer);
            break;
        }
        case AccountServiceMessageType::SaveAccountDataResponse: {
            handleResponse<SaveAccountDataResponse>(messageBuffer);
            break;
        }
        case AccountServiceMessageType::ServiceHeartbeatResponse: {
            // Nothing to do. Receiving any message resets our idle timeout.
            break;
        }
        default: {
            LOG_INFO("Received unexpected AccountServer message type: %u",
                     static_cast<unsigned int>(messageType));
            disconnectWithError(
                asio::error::make_error_code(asio::error::invalid_argument));
            break;
        }
    }
}

template<typename Response>
void AccountServiceEndpoint::handleResponse(
    std::span<const Uint8> messageBuffer)
{
    Response response{};
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 response)) {
        LOG_INFO("Failed to deserialize an AccountServer message.");
        disconnectWithError(
            asio::error::make_error_code(asio::error::invalid_argument));
        return;
    }

    completeRequest(response.requestID, response);
}

} // namespace AM
