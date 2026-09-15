#include "AccountHelpers.h"
#include "AccountDefs.h"
#include "StringTools.h"

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

    if (!StringTools::isAscii(username)) {
        return ValidateResult::InvalidCharacter;
    }

    return ValidateResult::Success;
}

// TODO: This error reporting will probably have to change, so we can show all 
//       the issues at once
AccountHelpers::ValidateResult
    AccountHelpers::validatePassword(std::string_view password)
{
    if (password.length() < static_cast<std::size_t>(PASSWORD_MIN_LENGTH)) {
        return ValidateResult::TooShort;
    }
    if (password.length() > static_cast<std::size_t>(PASSWORD_MAX_LENGTH)) {
        return ValidateResult::TooLong;
    }

    if (!StringTools::isAscii(password)) {
        return ValidateResult::InvalidCharacter;
    }

    return ValidateResult::Success;
}

} // End namespace AccountServer
} // End namespace AM
