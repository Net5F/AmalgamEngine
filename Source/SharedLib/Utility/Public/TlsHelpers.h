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
    /**
     * Loads and returns the TLS pin at the given path.
     * @return A valid pin, or null if the pin was not found or invalid. If 
     * returning null, an appropriate error message will be printed.
     */
    static std::optional<CertificatePin>
        loadCertificatePin(const std::string& pinPath);

    /**
     * Verifies that the given pin is valid within the given context.
     * @return true if the pin is valid, else false.
     */
    static bool verifyPinnedCertificate(
        const CertificatePin& expectedPin,
        asio::ssl::verify_context& verifyContext) noexcept;
};

} // End namespace AM
