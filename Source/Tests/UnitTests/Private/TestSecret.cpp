#include "catch2/catch_all.hpp"
#include "Secret.h"
#include "SecretString.h"
#include "Serialize.h"
#include "Deserialize.h"
#include <algorithm>
#include <array>
#include <new>
#include <string>
#include <vector>

using namespace AM;

namespace
{
using Token = std::array<Uint8, 8>;
using TestSecretString = SecretString<16>;

/** Messages for comparing SecretString's wire format against std::string's. */
struct SecretMessage {
    TestSecretString text{};
};
template<typename S>
void serialize(S& serializer, SecretMessage& message)
{
    serializer.text1b(message.text, TestSecretString::capacity());
}

struct PlainMessage {
    std::string text{};
};
template<typename S>
void serialize(S& serializer, PlainMessage& message)
{
    serializer.text1b(message.text, TestSecretString::capacity());
}

template<typename T>
std::vector<Uint8> toBytes(T& message)
{
    std::vector<Uint8> bytes(Serialize::measureSize(message));
    Serialize::toBuffer(bytes.data(), bytes.size(), message);
    return bytes;
}

bool isZeroed(const void* data, std::size_t size)
{
    const Uint8* bytes{static_cast<const Uint8*>(data)};
    return std::all_of(bytes, bytes + size, [](Uint8 b) { return b == 0; });
}
} // namespace

TEST_CASE("Secret")
{
    const Token token{1, 2, 3, 4, 5, 6, 7, 8};

    SECTION("Copy keeps both values")
    {
        Secret<Token> original{token};
        Secret<Token> copy{original};
        REQUIRE(original.get() == token);
        REQUIRE(copy.get() == token);
    }

    SECTION("Move construction wipes the source")
    {
        Secret<Token> original{token};
        Secret<Token> moved{std::move(original)};
        REQUIRE(moved.get() == token);
        REQUIRE(original.get() == Token{});
    }

    SECTION("Move assignment wipes the source")
    {
        Secret<Token> original{token};
        Secret<Token> moved{};
        moved = std::move(original);
        REQUIRE(moved.get() == token);
        REQUIRE(original.get() == Token{});
    }

    SECTION("Destruction wipes the value")
    {
        alignas(Secret<Token>) Uint8 storage[sizeof(Secret<Token>)];
        Secret<Token>* secret{new (storage) Secret<Token>{token}};
        REQUIRE(!isZeroed(storage, sizeof(storage)));
        secret->~Secret<Token>();
        REQUIRE(isZeroed(storage, sizeof(storage)));
    }
}

TEST_CASE("SecretString")
{
    SECTION("Assign and view")
    {
        TestSecretString secret{"password123"};
        REQUIRE(std::string{secret.view()} == "password123");
        REQUIRE(secret.size() == 11);
        REQUIRE(std::string{secret.c_str()} == "password123");
    }

    SECTION("Assign rejects text past capacity")
    {
        TestSecretString secret{"short"};
        REQUIRE(secret.assign(std::string(16, 'a')));
        REQUIRE(!secret.assign(std::string(17, 'b')));
        REQUIRE(std::string{secret.view()} == std::string(16, 'a'));
    }

    SECTION("Shorter assign wipes the old tail")
    {
        TestSecretString secret{"longpassword"};
        REQUIRE(secret.assign("abc"));
        REQUIRE(std::string{secret.view()} == "abc");
        REQUIRE(isZeroed(secret.data() + 3, TestSecretString::capacity() - 3));
    }

    SECTION("Clear wipes the text")
    {
        TestSecretString secret{"password123"};
        secret.clear();
        REQUIRE(secret.empty());
        REQUIRE(isZeroed(secret.data(), TestSecretString::capacity()));
    }

    SECTION("Move construction wipes the source")
    {
        TestSecretString original{"password123"};
        TestSecretString moved{std::move(original)};
        REQUIRE(std::string{moved.view()} == "password123");
        REQUIRE(original.empty());
        REQUIRE(isZeroed(original.data(), TestSecretString::capacity()));
    }

    SECTION("Move assignment wipes the source")
    {
        TestSecretString original{"password123"};
        TestSecretString moved{"other"};
        moved = std::move(original);
        REQUIRE(std::string{moved.view()} == "password123");
        REQUIRE(original.empty());
        REQUIRE(isZeroed(original.data(), TestSecretString::capacity()));
    }

    SECTION("Destruction wipes the text")
    {
        alignas(TestSecretString) Uint8 storage[sizeof(TestSecretString)];
        TestSecretString* secret{new (storage) TestSecretString{"password123"}};
        const char* chars{secret->data()};
        secret->~TestSecretString();
        REQUIRE(isZeroed(chars, TestSecretString::capacity() + 1));
    }

    SECTION("Wire format matches std::string")
    {
        SecretMessage secretMessage{TestSecretString{"password123"}};
        PlainMessage plainMessage{"password123"};
        REQUIRE(toBytes(secretMessage) == toBytes(plainMessage));
    }

    SECTION("Deserializes from std::string's format")
    {
        PlainMessage plainMessage{"password123"};
        std::vector<Uint8> bytes{toBytes(plainMessage)};

        SecretMessage secretMessage{};
        REQUIRE(
            Deserialize::fromBuffer(bytes.data(), bytes.size(), secretMessage));
        REQUIRE(std::string{secretMessage.text.view()} == "password123");
    }

    SECTION("Deserialization rejects text past the max size")
    {
        // Build the bytes by hand, since the serializer would assert.
        // Note: bitsery writes sizes below 128 as a single byte.
        std::vector<Uint8> bytes(18, 'a');
        bytes[0] = 17;

        SecretMessage secretMessage{};
        REQUIRE(!Deserialize::fromBuffer(bytes.data(), bytes.size(),
                                         secretMessage));
        REQUIRE(secretMessage.text.empty());
    }
}
