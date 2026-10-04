#include "CryptoHelpers.h"
#include "AccountDefs.h"
#include "sodium.h"
#include <array>

namespace AM
{
namespace AccountServer
{

/** Encoding 18 random bytes as unpadded Base64URL produces a 24-character
    recovery key with 144 bits of entropy. */
static constexpr std::size_t RECOVERY_KEY_RANDOM_BYTES{18};
static_assert(
    sodium_base64_ENCODED_LEN(RECOVERY_KEY_RANDOM_BYTES,
                              sodium_base64_VARIANT_URLSAFE_NO_PADDING)
    == (RECOVERY_KEY_CHARACTERS + 1));

void CryptoHelpers::fillRandomBytes(std::span<Uint8> buffer)
{
    randombytes_buf(buffer.data(), buffer.size());
}

std::string CryptoHelpers::generateRecoveryKey()
{
    std::array<unsigned char, RECOVERY_KEY_RANDOM_BYTES> keyBytes{};
    randombytes_buf(keyBytes.data(), keyBytes.size());

    // Note: The encoded length includes the null terminator.
    std::array<char, RECOVERY_KEY_CHARACTERS + 1> encodedKey{};
    sodium_bin2base64(encodedKey.data(), encodedKey.size(), keyBytes.data(),
                      keyBytes.size(),
                      sodium_base64_VARIANT_URLSAFE_NO_PADDING);

    // The encoded key is retained, so erase the raw copy.
    sodium_memzero(keyBytes.data(), keyBytes.size());

    return std::string{encodedKey.data(), RECOVERY_KEY_CHARACTERS};
}

std::optional<std::string>
    CryptoHelpers::hashPassword(std::string_view password)
{
    std::array<char, crypto_pwhash_STRBYTES> encodedHash{};

    int result{crypto_pwhash_str_alg(
        encodedHash.data(), password.data(),
        static_cast<unsigned long long>(password.size()),
        crypto_pwhash_OPSLIMIT_MODERATE, crypto_pwhash_MEMLIMIT_MODERATE,
        crypto_pwhash_ALG_ARGON2ID13)};
    if (result != 0) {
        // Usually means the requested memory couldn't be allocated.
        return std::nullopt;
    }

    return std::string{encodedHash.data()};
}

bool CryptoHelpers::verifyPassword(const std::string& passwordHash,
                                   std::string_view password)
{
    return crypto_pwhash_str_verify(
               passwordHash.c_str(), password.data(),
               static_cast<unsigned long long>(password.size()))
           == 0;
}

std::optional<std::string>
    CryptoHelpers::hashSecret(std::span<const Uint8> secret)
{
    std::array<unsigned char, SECRET_HASH_BYTES> hash{};

    int result{crypto_generichash(
        hash.data(), hash.size(), secret.data(),
        static_cast<unsigned long long>(secret.size()), nullptr, 0)};
    if (result != 0) {
        return std::nullopt;
    }

    // Use the explicit-length constructor because the binary digest can
    // contain null bytes.
    return std::string{reinterpret_cast<const char*>(hash.data()), hash.size()};
}

std::optional<std::string> CryptoHelpers::hashSecret(std::string_view secret)
{
    return hashSecret(std::span<const Uint8>{
        reinterpret_cast<const Uint8*>(secret.data()), secret.size()});
}

} // End namespace AccountServer
} // End namespace AM
