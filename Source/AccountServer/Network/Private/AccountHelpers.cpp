#include "AccountHelpers.h"
#include "AccountDefs.h"

namespace AM
{
namespace AccountServer
{

AccountHelpers::ValidateResult
    AccountHelpers::validateUsername(std::string_view username)
{
    if (username.length() < static_cast<std::size_t>(USERNAME_MIN_LENGTH)) {
        return ValidateResult::TooShort;
    }
    if (username.length() > static_cast<std::size_t>(USERNAME_MAX_LENGTH)) {
        return ValidateResult::TooLong;
    }

    if (!isAscii(username)) {
        return ValidateResult::InvalidCharacter;
    }

    return ValidateResult::Success;
}

AccountHelpers::ValidateResult
    AccountHelpers::validatePassword(std::string_view password)
{
    if (password.length() < static_cast<std::size_t>(PASSWORD_MIN_LENGTH)) {
        return ValidateResult::TooShort;
    }
    if (password.length() > static_cast<std::size_t>(PASSWORD_MAX_LENGTH)) {
        return ValidateResult::TooLong;
    }

    if (!isAscii(password)) {
        return ValidateResult::InvalidCharacter;
    }

    return ValidateResult::Success;
}

bool AccountHelpers::isAscii(std::string_view stringToCheck)
{
    for (char character : stringToCheck) {
        unsigned char castChar{static_cast<unsigned char>(character)};
        if (castChar > 0x7F || castChar < 0x20) {
            return false;
        }
    }

    return true;
}

} // End namespace AccountServer
} // End namespace AM
