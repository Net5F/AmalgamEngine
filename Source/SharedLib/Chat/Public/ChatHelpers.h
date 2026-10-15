#pragma once

#include <string_view>

namespace AM
{

/**
 * Helper functions for chat operations.
 */
class ChatHelpers
{
public:
    struct ValidateResult {
        bool empty{};
        bool tooLong{};
        bool invalidCharacter{};

        [[nodiscard]] bool success() const noexcept
        {
            return !empty && !tooLong && !invalidCharacter;
        }
    };
    /**
     * Chat text must contain at least 1 non-space character, must be no longer
     * than CHAT_MESSAGE_MAX_LENGTH, and must only contain printable ASCII
     * characters.
     *
     * @return Success if the given chat text is valid, else returns the bit
     * flags of all issues.
     */
    static ValidateResult validateChatText(std::string_view text);
};

} // End namespace AM
