#include "TlsHelpers.h"
#include "StringTools.h"
#include "Log.h"
#include "asio/ssl/verify_context.hpp"
#include "asio/system_error.hpp"
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>

namespace AM
{

std::optional<TlsHelpers::CertificatePin>
    TlsHelpers::loadCertificatePin(const std::string& pinPath)
{
    std::ifstream pinFile{pinPath, std::ios::binary};
    if (!pinFile.is_open()) {
        LOG_INFO("Failed to open certificate pin '%s'.", pinPath.c_str());
        return {};
    }

    std::string encodedPin{std::istreambuf_iterator<char>{pinFile},
                           std::istreambuf_iterator<char>{}};
    if (encodedPin.ends_with('\n')) {
        encodedPin.pop_back();
        if (encodedPin.ends_with('\r')) {
            encodedPin.pop_back();
        }
    }

    bool hasValidCharacters{(encodedPin.size() == SHA256_BASE64_LENGTH)
                            && (encodedPin.back() == '=')
                            && StringTools::isBase64(encodedPin)};
    if (!hasValidCharacters) {
        LOG_INFO("Certificate pin '%s' is malformed. Expected "
                 "one padded base64-encoded SHA-256 pin.",
                 pinPath.c_str());
        return {};
    }

    std::array<unsigned char, (SHA256_BASE64_LENGTH / 4) * 3> decodedPin{};
    int decodedLength{EVP_DecodeBlock(
        decodedPin.data(),
        reinterpret_cast<const unsigned char*>(encodedPin.data()),
        static_cast<int>(encodedPin.size()))};
    // EVP_DecodeBlock includes bytes represented by base64 padding.
    if ((decodedLength < 1) || ((decodedLength - 1) != SHA256_BYTES)) {
        LOG_INFO("Certificate pin '%s' is malformed. Expected "
                 "one padded base64-encoded SHA-256 pin.",
                 pinPath.c_str());
        return {};
    }

    CertificatePin certificatePin{};
    std::ranges::copy_n(decodedPin.begin(), certificatePin.size(),
                        certificatePin.begin());

    std::array<unsigned char, SHA256_BASE64_LENGTH + 1> canonicalPin{};
    int canonicalLength{
        EVP_EncodeBlock(canonicalPin.data(), certificatePin.data(),
                        static_cast<int>(certificatePin.size()))};
    if (canonicalLength != SHA256_BASE64_LENGTH) {
        LOG_INFO("Failed to validate certificate pin '%s'.", pinPath.c_str());
        return {};
    }
    std::string canonicalPinText{
        reinterpret_cast<const char*>(canonicalPin.data()),
        static_cast<std::size_t>(canonicalLength)};
    if (canonicalPinText != encodedPin) {
        LOG_INFO("Certificate pin '%s' is malformed. Expected one padded "
                 "base64-encoded SHA-256 pin.",
                 pinPath.c_str());
        return {};
    }

    return certificatePin;
}

bool TlsHelpers::verifyPinnedCertificate(
    const CertificatePin& expectedPin,
    asio::ssl::verify_context& verifyContext) noexcept
{
    X509_STORE_CTX* storeContext{verifyContext.native_handle()};
    if (storeContext == nullptr) {
        return false;
    }

    // Always inspect the target (leaf) certificate. OpenSSL may invoke this
    // callback while processing any certificate in the presented chain.
    X509* certificate{X509_STORE_CTX_get0_cert(storeContext)};
    if (certificate == nullptr) {
        return false;
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> publicKeyDigest{};
    unsigned int digestLength{};
    if ((X509_pubkey_digest(certificate, EVP_sha256(), publicKeyDigest.data(),
                            &digestLength)
         != 1)
        || (digestLength != expectedPin.size())
        || (CRYPTO_memcmp(publicKeyDigest.data(), expectedPin.data(),
                          expectedPin.size())
            != 0)) {
        return false;
    }

    // X509_cmp_current_time returns -1 for a time at/before now, 1 for a
    // future time, and 0 for a malformed value.
    if ((X509_cmp_current_time(X509_get0_notBefore(certificate)) != -1)
        || (X509_cmp_current_time(X509_get0_notAfter(certificate)) != 1)) {
        return false;
    }

    if (X509_check_purpose(certificate, X509_PURPOSE_SSL_SERVER, 0) != 1) {
        return false;
    }

    // The exact SPKI pin is our trust anchor, so a conventional CA chain is
    // not required. Clear any self-signed/untrusted-chain error after all
    // required leaf checks pass.
    X509_STORE_CTX_set_error(storeContext, X509_V_OK);
    return true;
}

} // End namespace AM
