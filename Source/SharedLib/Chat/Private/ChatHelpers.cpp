#include "ChatHelpers.h"
#include "ChatDefs.h"
#include "StringTools.h"

namespace AM
{

ChatHelpers::ValidateResult ChatHelpers::validateChatText(std::string_view text)
{
    ValidateResult result{};
    if (text.find_first_not_of(' ') == std::string_view::npos) {
        result.empty = true;
    }
    if (text.length() > CHAT_MESSAGE_MAX_LENGTH) {
        result.tooLong = true;
    }

    if (!StringTools::isAscii(text)) {
        result.invalidCharacter = true;
    }

    return result;
}

} // End namespace AM
