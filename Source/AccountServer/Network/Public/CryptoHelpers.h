#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace AM
{
namespace AccountServer
{

/**
 * Helper functions for generating and hashing account secrets (passwords,
 * recovery keys, session tokens, and service tickets).
 *
 * Note: libsodium must be initialized before calling these.
 */
class CryptoHelpers
{
public:
    /** The length, in bytes, of a hash returned by hashSecret().
        Must match the length of the hash columns in the database. */
    static constexpr std::size_t SECRET_HASH_BYTES{32};

    /**
     * Fills the given buffer with cryptographically random bytes.
     */
    static void fillRandomBytes(std::span<Uint8> buffer);

    /**
     * Generates an account recovery key, RECOVERY_KEY_CHARACTERS long.
     */
    static std::string generateRecoveryKey();

    /**
     * Hashes the given password using argon2id.
     * @return The encoded hash, or null if hashing failed.
     */
    static std::optional<std::string> hashPassword(std::string_view password);

    /**
     * @return true if the given password matches the given encoded hash, else
     *         false.
     */
    static bool verifyPassword(const std::string& passwordHash,
                               std::string_view password);

    /**
     * Hashes a high-entropy secret (recovery key, session token, or service
     * ticket) into exactly SECRET_HASH_BYTES binary bytes.
     *
     * Note: This hash is fast, so it must not be used for passwords.
     *
     * @return The binary hash, or null if hashing failed.
     */
    static std::optional<std::string> hashSecret(std::span<const Uint8> secret);

    /**
     * Overload for text secrets.
     */
    static std::optional<std::string> hashSecret(std::string_view secret);
};

} // End namespace AccountServer
} // End namespace AM
