#pragma once

#include <string_view>

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
    enum class ValidateResult {
        Success,
        TooShort,
        TooLong,
        InvalidCharacter
    };
    /**
     * @return Success if the given username is valid.
     */
    static ValidateResult validateUsername(std::string_view username);

    /**
     * @return Success if the given password is valid.
     */
    static ValidateResult validatePassword(std::string_view password);
};

} // End namespace AccountServer
} // End namespace AM
