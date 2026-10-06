#include "Database.h"
#include "CryptoHelpers.h"
#include "AccountHelpers.h"
#include "LoadTestAccounts.h"
#include "sodium.h"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>

using namespace AM;
using namespace AM::AccountServer;
using namespace AM::LTC;

void printUsage()
{
    std::printf(
        "Usage: SeedLoadTestAccounts <AccountsDbPath> <NumAccounts>\n"
        "  AccountsDbPath: The path to the AccountServer's Accounts.db.\n"
        "  NumAccounts: How many load test accounts should exist.\n"
        "\n"
        "Creates the accounts %s0 through %s<NumAccounts - 1>, all with the "
        "password \"%s\".\n"
        "Accounts that already exist are left as-is.\n",
        LOAD_TEST_USERNAME_PREFIX.data(), LOAD_TEST_USERNAME_PREFIX.data(),
        LOAD_TEST_PASSWORD.data());
}

int main(int argc, char** argv)
try {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    // Check that the database exists, so a typo doesn't silently create a
    // new one.
    std::filesystem::path databasePath{argv[1]};
    if (!std::filesystem::exists(databasePath)) {
        std::printf("Database not found: %s\n"
                    "Run AccountServer once to create it.\n",
                    argv[1]);
        return 1;
    }

    // Parse NumAccounts.
    char* end;
    long input{std::strtol(argv[2], &end, 10)};
    if ((*end != '\0') || (input < 1)) {
        std::printf("Invalid NumAccounts: %s\n", argv[2]);
        printUsage();
        return 1;
    }
    unsigned int numAccounts{static_cast<unsigned int>(input)};

    if (sodium_init() < 0) {
        std::printf("Failed to initialize libsodium.\n");
        return 1;
    }

    // Note: All load test accounts share a password, so we hash it once and
    //       reuse the hash. Password hashing is deliberately slow, so this
    //       saves a lot of time when seeding many accounts.
    std::optional<std::string> passwordHash{
        CryptoHelpers::hashPassword(LOAD_TEST_PASSWORD)};
    if (!passwordHash) {
        std::printf("Failed to hash password.\n");
        return 1;
    }

    Database database{databasePath.string()};

    unsigned int createdCount{0};
    unsigned int existingCount{0};
    unsigned int failedCount{0};
    for (unsigned int i{0}; i < numAccounts; ++i) {
        std::string username{getLoadTestUsername(i)};
        if (!(AccountHelpers::validateUsername(username).success())) {
            std::printf("Generated invalid username: %s\n", username.c_str());
            failedCount++;
            continue;
        }

        // Every account needs a recovery key, but nobody will use these, so
        // we just store the hash and discard the key.
        std::optional<std::string> recoveryKeyHash{
            CryptoHelpers::hashSecret(CryptoHelpers::generateRecoveryKey())};
        if (!recoveryKeyHash) {
            std::printf("Failed to hash recovery key for: %s\n",
                        username.c_str());
            failedCount++;
            continue;
        }

        switch (database.registerAccount(username, *passwordHash,
                                         *recoveryKeyHash)) {
            case Database::RegisterResult::Success: {
                createdCount++;
                break;
            }
            case Database::RegisterResult::UsernameUnavailable: {
                existingCount++;
                break;
            }
            case Database::RegisterResult::DatabaseError: {
                std::printf("Failed to register: %s\n", username.c_str());
                failedCount++;
                break;
            }
        }
    }

    std::printf("Created %u accounts, %u already existed, %u failed.\n",
                createdCount, existingCount, failedCount);

    return (failedCount == 0) ? 0 : 1;
} catch (std::exception& e) {
    std::printf("%s\n", e.what());
    return 1;
}
