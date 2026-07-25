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

private:
    /** Returns true if every character in the given string is within the 
        printable ASCII character set. */
    static bool isAscii(std::string_view string);
};

} // End namespace AccountServer
} // End namespace AM
