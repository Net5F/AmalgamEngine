#pragma once

#include <SDL3/SDL_stdinc.h>
#include <string_view>
#include <utility>

namespace AM
{
namespace AccountServer
{

/**
 * Helper functions for account operations.
 */
class AccountHelpers
{
public:
    struct ValidateResult {
        bool tooShort{};
        bool tooLong{};
        bool invalidCharacter{};

        [[nodiscard]] bool success() const noexcept
        {
            return !tooShort && !tooLong && !invalidCharacter;
        }
    };
    /**
     * @return Success if the given username is valid, else returns the bit
     * flags of all issues.
     */
    static ValidateResult validateUsername(std::string_view username);

    /**
     * @return Success if the given password is valid, else returns the bit
     * flags of all issues.
     */
    static ValidateResult validatePassword(std::string_view password);
};

} // End namespace AccountServer
} // End namespace AM
