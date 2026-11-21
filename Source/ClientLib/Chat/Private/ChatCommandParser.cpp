#include "ChatCommandParser.h"

namespace AM
{
namespace Client
{

std::optional<ChatCommandParser::ParsedCommand>
    ChatCommandParser::parse(std::string_view text)
{
    auto [name, arguments] = splitFirstToken(text);
    if (!name.starts_with('/')) {
        return std::nullopt;
    }

    return ParsedCommand{name, arguments};
}

std::pair<std::string_view, std::string_view>
    ChatCommandParser::splitFirstToken(std::string_view text)
{
    std::size_t tokenStart{text.find_first_not_of(' ')};
    if (tokenStart == std::string_view::npos) {
        return {};
    }
    text.remove_prefix(tokenStart);

    std::size_t tokenEnd{text.find(' ')};
    if (tokenEnd == std::string_view::npos) {
        return {text, {}};
    }

    std::string_view remainder{text.substr(tokenEnd)};
    std::size_t remainderStart{remainder.find_first_not_of(' ')};
    if (remainderStart == std::string_view::npos) {
        remainder = {};
    }
    else {
        remainder.remove_prefix(remainderStart);
    }

    return {text.substr(0, tokenEnd), remainder};
}

} // namespace Client
} // namespace AM
