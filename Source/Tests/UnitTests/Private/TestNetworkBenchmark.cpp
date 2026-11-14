#include "catch2/catch_all.hpp"
#include "SimpleConnection.h"
#include "AccountClientMessageType.h"
#include "AccountServiceMessageType.h"
#include "ChatClientMessageType.h"
#include "LoginRequest.h"
#include "ChatSendRequest.h"
#include "SaveAccountDataRequest.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/context.hpp"
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <memory>
#include <stdexcept>

/**
 * Benchmarks for the SimpleConnection send/receive path.
 *
 * Each benchmark sends a batch of messages from a client connection to a
 * server connection over TLS on localhost, and measures how long it takes for
 * the whole batch to be received. Both ends run on the benchmark thread.
 *
 * The receive callback only counts messages (it doesn't deserialize them), so
 * any fixed per-message cost is a larger share of the total than it would be
 * in a real server.
 *
 * These are hidden, run them with: UnitTests "[benchmark]"
 */

using namespace AM;

namespace
{
/**
 * Adds a throwaway self-signed certificate to the given server context.
 */
void addSelfSignedCertificate(asio::ssl::context& serverContext)
{
    EVP_PKEY* key{EVP_EC_gen("P-256")};
    X509* certificate{X509_new()};
    if (!key || !certificate) {
        throw std::runtime_error("Failed to create benchmark certificate.");
    }

    X509_set_version(certificate, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(certificate), 1);
    X509_gmtime_adj(X509_getm_notBefore(certificate), 0);
    X509_gmtime_adj(X509_getm_notAfter(certificate), 60 * 60);
    X509_set_pubkey(certificate, key);
    X509_NAME* name{X509_get_subject_name(certificate)};
    X509_NAME_add_entry_by_txt(
        name, "CN", MBSTRING_ASC,
        reinterpret_cast<const unsigned char*>("localhost"), -1, -1, 0);
    X509_set_issuer_name(certificate, name);
    X509_sign(certificate, key, EVP_sha256());

    SSL_CTX* nativeContext{serverContext.native_handle()};
    if ((SSL_CTX_use_certificate(nativeContext, certificate) != 1)
        || (SSL_CTX_use_PrivateKey(nativeContext, key) != 1)) {
        throw std::runtime_error("Failed to use benchmark certificate.");
    }

    X509_free(certificate);
    EVP_PKEY_free(key);
}

/**
 * A connected client/server pair of SimpleConnections over TLS on localhost.
 */
template<typename MessageType>
struct ConnectionPair {
    using Connection = SimpleConnection<MessageType>;

    asio::io_context ioContext{};
    asio::ssl::context serverContext{asio::ssl::context::tls_server};
    asio::ssl::context clientContext{asio::ssl::context::tls_client};
    std::shared_ptr<Connection> server{};
    std::shared_ptr<Connection> client{};
    std::size_t receivedCount{0};

    explicit ConnectionPair(Uint16 maxPayloadSize)
    {
        addSelfSignedCertificate(serverContext);
        clientContext.set_verify_mode(asio::ssl::verify_none);

        // Connect a socket pair.
        asio::ip::tcp::acceptor acceptor{
            ioContext,
            {asio::ip::make_address("127.0.0.1"), 0}};
        asio::ip::tcp::socket clientSocket{ioContext};
        clientSocket.connect(acceptor.local_endpoint());
        asio::ip::tcp::socket serverSocket{acceptor.accept()};

        // Allow a whole batch to be queued at once.
        typename Connection::ClassConfig config{
            .tlsTransportConfig{.handshakeTimeoutS{10},
                                .maxQueuedWriteBytes{16 * 1024 * 1024},
                                .maxQueuedMessages{4096}},
            .partialReceiveTimeoutS{10},
            .idleTimeoutS{60},
            .maxReadPayloadSize{maxPayloadSize},
            .maxWritePayloadSize{maxPayloadSize}};
        server = std::make_shared<Connection>(std::move(serverSocket),
                                              serverContext, config);
        client = std::make_shared<Connection>(std::move(clientSocket),
                                              clientContext, config);

        server->setMessageCallback(
            [this](MessageType, std::span<const Uint8>) { receivedCount++; });
        client->setMessageCallback([](MessageType, std::span<const Uint8>) {});

        // Run the handshakes.
        auto onDisconnect{[](const asio::error_code& error) {
            FAIL("Benchmark connection disconnected: " << error.message());
        }};
        server->startServer([](AM::Connection&) {}, onDisconnect);
        client->startClient([](AM::Connection&) {}, onDisconnect);
        while (!(server->isReady() && client->isReady())) {
            ioContext.run_one();
        }
    }

    ~ConnectionPair()
    {
        client->close();
        server->close();
    }

    /**
     * Sends the given message batchSize times, and waits for them all to be
     * received.
     */
    template<typename Message>
    std::size_t sendBatch(const Message& message, std::size_t batchSize)
    {
        std::size_t targetCount{receivedCount + batchSize};
        for (std::size_t i{0}; i < batchSize; ++i) {
            client->send(message);
        }
        while (receivedCount < targetCount) {
            if (ioContext.run_one() == 0) {
                FAIL("Benchmark IO context ran out of work.");
            }
        }
        return receivedCount;
    }
};
} // namespace

TEST_CASE("SimpleConnection throughput", "[.][benchmark]")
{
    SECTION("Small secret messages (LoginRequest)")
    {
        ConnectionPair<AccountClientMessageType> pair{500};
        LoginRequest loginRequest{.username{"benchmark-user"},
                                  .password{"benchmark-password"}};

        BENCHMARK("1000 LoginRequests")
        {
            return pair.sendBatch(loginRequest, 1000);
        };
    }

    SECTION("Small non-secret messages (ChatSendRequest)")
    {
        ConnectionPair<ChatClientMessageType> pair{500};
        ChatSendRequest chatSendRequest{.text{std::string(100, 'a')}};

        BENCHMARK("1000 ChatSendRequests")
        {
            return pair.sendBatch(chatSendRequest, 1000);
        };
    }

    SECTION("Large non-secret messages (SaveAccountDataRequest)")
    {
        ConnectionPair<AccountServiceMessageType> pair{65535};
        SaveAccountDataRequest saveRequest{};
        saveRequest.accountData.engineComponents.assign(20000, 1);
        saveRequest.accountData.projectComponents.assign(10000, 2);

        BENCHMARK("100 30KB SaveAccountDataRequests")
        {
            return pair.sendBatch(saveRequest, 100);
        };
    }
}
