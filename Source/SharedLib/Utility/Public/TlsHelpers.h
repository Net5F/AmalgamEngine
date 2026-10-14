#pragma once

#include <string>
#include <optional>
#include <array>

namespace asio
{
namespace ssl
{
class verify_context;
}
}

// OpenSSL's SSL and X509 types.
struct ssl_st;
struct x509_st;

/**
 * Static functions for working with the asio TLS interface.
 */
namespace AM
{
class TlsHelpers
{
public:
    /** The length, in bytes, of a SHA256 string. */
    static constexpr std::size_t SHA256_BYTES{32};
    /** The length, in bytes, of a SHA256 string encoded in base 64. */
    static constexpr std::size_t SHA256_BASE64_LENGTH{44};

    using CertificatePin = std::array<unsigned char, SHA256_BYTES>;

    /** The role that a pinned peer is expected to play in the connection. */
    enum class PeerRole {
        /** The peer is a server that we connected to. */
        Server,
        /** The peer is a client that connected to us (mutual TLS). */
        Client
    };
    /**
     * Loads and returns the TLS pin at the given path.
     * @return A valid pin, or null if the pin was not found or invalid. If 
     * returning null, an appropriate error message will be printed.
     */
    static std::optional<CertificatePin>
        loadCertificatePin(const std::string& pinPath);

    /**
     * Verifies that the given pin is valid within the given context.
     *
     * @param peerRole The role that the peer is expected to play. Used to
     *                 check that the certificate is authorized for that use.
     * @return true if the pin is valid, else false.
     */
    static bool verifyPinnedCertificate(
        const CertificatePin& expectedPin,
        asio::ssl::verify_context& verifyContext,
        PeerRole peerRole = PeerRole::Server) noexcept;

    /**
     * Returns the pin of the certificate that the peer presented during the
     * given connection's handshake.
     *
     * Note: This doesn't verify the certificate. It's intended for telling
     *       apart peers that were already verified during the handshake
     *       (e.g. by verifyPinnedCertificate()).
     *
     * @return The peer's pin, or null if the peer didn't present a
     *         certificate or the pin couldn't be computed.
     */
    static std::optional<CertificatePin>
        getPeerCertificatePin(ssl_st* ssl) noexcept;

private:
    /**
     * Returns the pin (SHA-256 digest of the public key info) of the given
     * certificate, or null if it couldn't be computed.
     */
    static std::optional<CertificatePin>
        computeCertificatePin(x509_st* certificate) noexcept;
};

} // End namespace AM
