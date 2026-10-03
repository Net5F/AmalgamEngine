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
    ValidateResult result{};
    if (username.length() < static_cast<std::size_t>(USERNAME_MIN_LENGTH)) {
        result.tooShort = true;
    }

    if (!StringTools::isAscii(username)) {
        result.invalidCharacter = true;
    }

    return result;
}

AccountHelpers::ValidateResult
    AccountHelpers::validatePassword(std::string_view password)
{
    ValidateResult result{};
    if (password.length() < static_cast<std::size_t>(PASSWORD_MIN_LENGTH)) {
        result.tooShort = true;
    }
    if (password.length() > static_cast<std::size_t>(PASSWORD_MAX_LENGTH)) {
        result.tooLong = true;
    }

    if (!StringTools::isAscii(password)) {
        result.invalidCharacter = true;
    }

    return result;
}

} // End namespace AccountServer
} // End namespace AM
