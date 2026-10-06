#pragma once

#include <string>
#include <string_view>

namespace AM
{
namespace LTC
{
/**
 * Definitions for the accounts that the load test client logs in as.
 *
 * SeedLoadTestAccounts creates these accounts in an AccountServer's database,
 * and LoadTestClient logs in with them.
 *
 * Note: Since the password is public, never seed a production database.
 */

/** Load test usernames are this prefix, followed by the client's index. */
inline constexpr std::string_view LOAD_TEST_USERNAME_PREFIX{"loadtest_"};

/** The password that all load test accounts share. */
inline constexpr std::string_view LOAD_TEST_PASSWORD{"loadtest-password"};

/**
 * Returns the username of the load test account with the given index.
 */
inline std::string getLoadTestUsername(unsigned int index)
{
    return std::string{LOAD_TEST_USERNAME_PREFIX} + std::to_string(index);
}

} // End namespace LTC
} // End namespace AM
