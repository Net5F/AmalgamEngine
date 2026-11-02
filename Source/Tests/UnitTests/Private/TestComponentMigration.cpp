#include "catch2/catch_all.hpp"
#include "ComponentMigration.h"
#include "Serialize.h"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <span>

using namespace AM;
using namespace AM::WorldServer;

namespace
{
/** Current version: 1. v0 had only "value". */
struct SingleStepComponent {
    Uint32 value{};
    Uint16 extra{};
};

template<typename S>
void serialize(S& serializer, SingleStepComponent& component)
{
    serializer.value4b(component.value);
    serializer.value2b(component.extra);
}

/** Current version: 2. v0 had only "value", v1 added "extra". */
struct ChainedComponent {
    Uint32 value{};
    Uint16 extra{};
    Uint8 flag{};
};

template<typename S>
void serialize(S& serializer, ChainedComponent& component)
{
    serializer.value4b(component.value);
    serializer.value2b(component.extra);
    serializer.value1b(component.flag);
}

/** Current version: 1. Its v0 -> v1 migration always fails. */
struct FailingComponent {
    Uint32 value{};
};

template<typename S>
void serialize(S& serializer, FailingComponent& component)
{
    serializer.value4b(component.value);
}

/** Values that the migrations set, so we can tell that they ran. */
constexpr Uint16 MIGRATED_EXTRA{0xBEEF};
constexpr Uint8 MIGRATED_FLAG{0x7F};
} // namespace

namespace AM
{
namespace WorldServer
{
// SingleStepComponent v0 -> v1.
template<>
struct ComponentMigration<SingleStepComponent, 0> {
    struct OldComponent {
        Uint32 value{};

        template<typename S>
        void serialize(S& serializer)
        {
            serializer.value4b(value);
        }
    };

    static bool migrate(const OldComponent& oldComponent, auto& newComponent)
    {
        newComponent.value = oldComponent.value;
        newComponent.extra = MIGRATED_EXTRA;
        return true;
    }
};

// ChainedComponent v0 -> v1.
template<>
struct ComponentMigration<ChainedComponent, 0> {
    struct OldComponent {
        Uint32 value{};

        template<typename S>
        void serialize(S& serializer)
        {
            serializer.value4b(value);
        }
    };

    static bool migrate(const OldComponent& oldComponent, auto& newComponent)
    {
        newComponent.value = oldComponent.value;
        newComponent.extra = MIGRATED_EXTRA;
        return true;
    }
};

// ChainedComponent v1 -> v2.
template<>
struct ComponentMigration<ChainedComponent, 1> {
    struct OldComponent {
        Uint32 value{};
        Uint16 extra{};

        template<typename S>
        void serialize(S& serializer)
        {
            serializer.value4b(value);
            serializer.value2b(extra);
        }
    };

    static bool migrate(const OldComponent& oldComponent, auto& newComponent)
    {
        newComponent.value = oldComponent.value;
        newComponent.extra = oldComponent.extra;
        newComponent.flag = MIGRATED_FLAG;
        return true;
    }
};

// FailingComponent v0 -> v1.
template<>
struct ComponentMigration<FailingComponent, 0> {
    struct OldComponent {
        Uint32 value{};

        template<typename S>
        void serialize(S& serializer)
        {
            serializer.value4b(value);
        }
    };

    static bool migrate(const OldComponent&, auto&) { return false; }
};
} // namespace WorldServer
} // namespace AM

namespace
{
/** Serializes the given old component, the same way the engine saves it. */
template<typename T>
std::vector<Uint8> serializeOld(T component)
{
    std::array<Uint8, 64> buffer{};
    std::size_t size{
        Serialize::toBuffer(buffer.data(), buffer.size(), component)};
    return {buffer.begin(), buffer.begin() + size};
}
} // namespace

TEST_CASE("TestComponentMigration")
{
    SECTION("Single step v0 -> v1")
    {
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<SingleStepComponent, 0>::OldComponent{1234})};

        SingleStepComponent component{};
        REQUIRE(migrateComponent<SingleStepComponent, 1>(0, payload, component)
                == ComponentMigrationResult::Success);
        REQUIRE(component.value == 1234);
        REQUIRE(component.extra == MIGRATED_EXTRA);
    }

    SECTION("Chained v0 -> v2")
    {
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<ChainedComponent, 0>::OldComponent{5678})};

        ChainedComponent component{};
        REQUIRE(migrateComponent<ChainedComponent, 2>(0, payload, component)
                == ComponentMigrationResult::Success);
        REQUIRE(component.value == 5678);
        REQUIRE(component.extra == MIGRATED_EXTRA);
        REQUIRE(component.flag == MIGRATED_FLAG);
    }

    SECTION("Mid-chain v1 -> v2")
    {
        // Use a non-default extra, so we can tell that v0 -> v1 didn't run.
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<ChainedComponent, 1>::OldComponent{42, 7})};

        ChainedComponent component{};
        REQUIRE(migrateComponent<ChainedComponent, 2>(1, payload, component)
                == ComponentMigrationResult::Success);
        REQUIRE(component.value == 42);
        REQUIRE(component.extra == 7);
        REQUIRE(component.flag == MIGRATED_FLAG);
    }

    SECTION("Unsupported version")
    {
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<SingleStepComponent, 0>::OldComponent{1})};

        // The current version isn't a migration source.
        SingleStepComponent component{};
        REQUIRE(migrateComponent<SingleStepComponent, 1>(1, payload, component)
                == ComponentMigrationResult::UnsupportedVersion);

        // Neither is a version newer than the current one.
        REQUIRE(migrateComponent<SingleStepComponent, 1>(5, payload, component)
                == ComponentMigrationResult::UnsupportedVersion);
    }

    SECTION("Truncated payload")
    {
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<SingleStepComponent, 0>::OldComponent{1})};
        payload.pop_back();

        SingleStepComponent component{};
        REQUIRE(migrateComponent<SingleStepComponent, 1>(0, payload, component)
                == ComponentMigrationResult::DeserializationFailed);
    }

    SECTION("Payload with trailing bytes")
    {
        // A v1 payload labeled as v0 has 2 extra bytes, which must fail
        // instead of silently being ignored.
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<ChainedComponent, 1>::OldComponent{1, 2})};

        ChainedComponent component{};
        REQUIRE(migrateComponent<ChainedComponent, 2>(0, payload, component)
                == ComponentMigrationResult::DeserializationFailed);
    }

    SECTION("Migration function fails")
    {
        std::vector<Uint8> payload{serializeOld(
            ComponentMigration<FailingComponent, 0>::OldComponent{1})};

        FailingComponent component{};
        REQUIRE(migrateComponent<FailingComponent, 1>(0, payload, component)
                == ComponentMigrationResult::MigrationFailed);
    }
}
