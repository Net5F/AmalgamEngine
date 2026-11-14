#pragma once

#include "ByteTools.h"
#include "Log.h"
#include "bitsery/traits/core/std_defaults.h"
#include <array>
#include <cstddef>
#include <string_view>

namespace AM
{
/**
 * Holds a text secret, such as a password or recovery key, and wipes it from
 * memory when it's no longer needed.
 *
 * The text is stored inline in a fixed-capacity buffer, so it never touches
 * the heap and is never left behind by a reallocation.
 *
 * The buffer is wiped when this object is destroyed, when it's moved from, and
 * when it's cleared or shrunk. Copies are allowed (e.g. so the secret can be
 * passed through event queues), and each copy wipes itself in the same way.
 *
 * The text is always null-terminated, so c_str() can be passed to C APIs.
 *
 * Serialization: Use the same calls as std::string, with the capacity as the
 *                max size, e.g.
 *                "serializer.text1b(message.password, PASSWORD_MAX);"
 *                The wire format matches std::string's.
 *                The max size must not be larger than Capacity.
 *
 * @tparam Capacity The max number of characters this string can hold, not
 *                  including the null terminator.
 */
template<std::size_t Capacity>
class SecretString
{
public:
    // Container types, used by bitsery.
    using value_type = char;
    using difference_type = std::ptrdiff_t;
    using iterator = char*;
    using const_iterator = const char*;

    SecretString() noexcept
    : chars{}
    , length{0}
    {
    }

    /**
     * Note: Errors if the given text is longer than Capacity. Validate the
     *       length before constructing, or use assign() instead.
     */
    explicit SecretString(std::string_view inText)
    : SecretString{}
    {
        if (!assign(inText)) {
            LOG_FATAL("Text is longer than SecretString capacity.");
        }
    }

    SecretString(const SecretString& other) noexcept = default;

    SecretString(SecretString&& other) noexcept
    : chars{other.chars}
    , length{other.length}
    {
        other.clear();
    }

    SecretString& operator=(const SecretString& other) noexcept = default;

    SecretString& operator=(SecretString&& other) noexcept
    {
        if (this != &other) {
            chars = other.chars;
            length = other.length;
            other.clear();
        }
        return *this;
    }

    ~SecretString() { ByteTools::wipe(chars.data(), chars.size()); }

    /**
     * Replaces this string's text with the given text.
     *
     * @return false if the given text is longer than Capacity. In that case,
     *         this string is left unchanged.
     */
    bool assign(std::string_view inText) noexcept
    {
        if (inText.size() > Capacity) {
            return false;
        }

        // Copy the new text, then wipe any old text that's past its end.
        // Note: The rest of the buffer is already zeroed (see wipeTail()).
        std::size_t oldLength{length};
        inText.copy(chars.data(), inText.size());
        length = inText.size();
        if (oldLength > length) {
            wipeTail(oldLength);
        }

        return true;
    }

    /**
     * Wipes the text, leaving this string empty.
     */
    void clear() noexcept
    {
        ByteTools::wipe(chars.data(), chars.size());
        length = 0;
    }

    /**
     * Resizes the string to the given length. New characters are zeroed and
     * removed characters are wiped.
     *
     * Note: Errors if newLength is larger than Capacity.
     */
    void resize(std::size_t newLength)
    {
        if (newLength > Capacity) {
            LOG_FATAL("Tried to resize SecretString past its capacity.");
        }

        std::size_t oldLength{length};
        length = newLength;
        if (oldLength > newLength) {
            wipeTail(oldLength);
        }
    }

    std::string_view view() const noexcept { return {chars.data(), length}; }
    const char* c_str() const noexcept { return chars.data(); }
    char* data() noexcept { return chars.data(); }
    const char* data() const noexcept { return chars.data(); }
    std::size_t size() const noexcept { return length; }
    bool empty() const noexcept { return (length == 0); }
    static constexpr std::size_t capacity() noexcept { return Capacity; }

    iterator begin() noexcept { return chars.data(); }
    iterator end() noexcept { return chars.data() + length; }
    const_iterator begin() const noexcept { return chars.data(); }
    const_iterator end() const noexcept { return chars.data() + length; }

private:
    /**
     * Wipes the characters from the current length up to oldLength.
     */
    void wipeTail(std::size_t oldLength) noexcept
    {
        ByteTools::wipe(chars.data() + length, oldLength - length);
    }

    /** The text, followed by zeros.
        Every character past the text is kept zeroed, so the text is always
        null-terminated (hence the extra slot). */
    std::array<char, Capacity + 1> chars;

    /** The length of the text, not including the null terminator. */
    std::size_t length;
};

} // End namespace AM

namespace bitsery
{
namespace traits
{
/**
 * Lets bitsery serialize SecretString the same way as std::string.
 */
template<std::size_t Capacity>
struct ContainerTraits<AM::SecretString<Capacity>>
: public StdContainer<AM::SecretString<Capacity>, true, true> {
};

template<std::size_t Capacity>
struct TextTraits<AM::SecretString<Capacity>> {
    using TValue = char;
    // SecretString manages its own null terminator.
    static constexpr bool addNUL = false;

    static std::size_t length(const AM::SecretString<Capacity>& str)
    {
        return str.size();
    }
};

} // namespace traits
} // namespace bitsery
