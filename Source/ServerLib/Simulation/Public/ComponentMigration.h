#pragma once

#include "Deserialize.h"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <concepts>
#include <optional>
#include <span>
#include <utility>
#include <variant>

namespace AM
{
namespace Server
{

/**
 * A migration step for Component from BaseVersion to BaseVersion + 1.
 * Forward declaration for the individual dev-defined migration steps.
 *
 * Specializations must provide:
 * - An OldComponent type matching Component's serialized state at BaseVersion.
 * - A static migrate(const OldComponent&, NewComponent&) function that
 *   returns true on success.
 *
 * In migrate(), newComponent is the output of the migration step (either the
 * new final component, or the next component in the migration chain).
 */
template<typename Component, Uint16 BaseVersion>
struct ComponentMigration;

enum class ComponentMigrationResult {
    Success,
    UnsupportedVersion,
    DeserializationFailed,
    MigrationFailed
};

/**
 * Holds the component migration logic.
 *
 * Approach:
 *   1. Deserializes the component data into a State variable. State is a
 *      variant composed of the OldComponent types for each version before
 *      CurrentVersion.
 *   2. Runs each intermediate migration, storing the result back in the same
 *      State variable.
 *   3. Runs a final migration, storing the result in a Component variable.
 */
template<typename Component, Uint16 CurrentVersion>
class ComponentMigrator
{
    static_assert(CurrentVersion > 0);

public:
    /**
     * Migrates from serializedVersion to CurrentVersion.
     */
    static ComponentMigrationResult
        migrate(Uint16 serializedVersion,
                std::span<const Uint8> serializedComponent,
                Component& currentComponent)
    {
        if (serializedVersion >= CurrentVersion) {
            return ComponentMigrationResult::UnsupportedVersion;
        }

        std::optional<State> migrationState{};
        ComponentMigrationResult result{
            deserializationFunctions()[serializedVersion](
                serializedComponent, migrationState)};
        if (result != ComponentMigrationResult::Success) {
            return result;
        }
        else if (!(migrationState.has_value())) {
            return ComponentMigrationResult::MigrationFailed;
        }

        for (Uint16 version{serializedVersion};
             static_cast<Uint16>(version + 1) < CurrentVersion; ++version) {
            result = intermediateMigrationFunctions()[version](
                *migrationState);
            if (result != ComponentMigrationResult::Success) {
                return result;
            }
        }

        return migrateFinalStep(*migrationState, currentComponent);
    }

private:
    static constexpr std::size_t VERSION_COUNT{
        static_cast<std::size_t>(CurrentVersion)};
    static constexpr std::size_t INTERMEDIATE_MIGRATION_COUNT{
        (CurrentVersion > 0) ? static_cast<std::size_t>(CurrentVersion - 1)
                             : 0};

    /** Pulls the OldComponent type from the dev-defined migration function. */
    template<Uint16 BaseVersion>
    using OldComponent =
        typename ComponentMigration<Component, BaseVersion>::OldComponent;

    /** Together, these define a variant that contains the OldComponent type
        from each step in the migration.
        "State" in this context means "variant that holds the component data
        of the current step in the migration". */
    template<std::size_t... Indices>
    static auto makeStateType(std::index_sequence<Indices...>)
        -> std::variant<OldComponent<static_cast<Uint16>(Indices)>...>;
    using State =
        decltype(makeStateType(std::make_index_sequence<VERSION_COUNT>{}));

    /** Function pointer types. Used to store generated deserialize and
        migration functions for each step in the migration. */
    using DeserializationFunction = ComponentMigrationResult (*)(
        std::span<const Uint8> serializedComponent,
        std::optional<State>& migrationState);
    using IntermediateMigrationFunction =
        ComponentMigrationResult (*)(State& migrationState);

    // Validation helpers
    template<Uint16 BaseVersion>
    static constexpr bool hasMigration = requires {
        typename ComponentMigration<Component, BaseVersion>::OldComponent;
    };

    template<typename Migration, typename OldComponent, typename NextComponent>
    static constexpr bool validMigration = requires(
        const OldComponent& oldComponent, NextComponent& nextComponent) {
        {
            Migration::migrate(oldComponent, nextComponent)
        } -> std::same_as<bool>;
    };

    template<std::size_t... Indices>
    static consteval bool hasAllMigrations(std::index_sequence<Indices...>)
    {
        return (
            hasMigration<static_cast<Uint16>(Indices)> && ...);
    }

    static_assert(hasAllMigrations(
                      std::make_index_sequence<VERSION_COUNT>{}),
                  "Missing ComponentMigration specialization.");

    /**
     * Deserializes serializedComponent at the given version into
     * migrationState.
     */
    template<Uint16 BaseVersion>
    static ComponentMigrationResult
        deserializeVersion(std::span<const Uint8> serializedComponent,
                           std::optional<State>& migrationState)
    {
        using ComponentAtBaseVersion = OldComponent<BaseVersion>;

        ComponentAtBaseVersion component{};
        if (!Deserialize::fromBuffer(serializedComponent.data(),
                                     serializedComponent.size(), component)) {
            return ComponentMigrationResult::DeserializationFailed;
        }

        migrationState.emplace(
            std::in_place_index<static_cast<std::size_t>(BaseVersion)>,
            std::move(component));
        return ComponentMigrationResult::Success;
    }

    /**
     * Migrates migrationState one version forward.
     * @post migrationState contains the next version's component type.
     */
    template<Uint16 BaseVersion>
    static ComponentMigrationResult migrateIntermediateStep(State& migrationState)
    {
        constexpr Uint16 nextVersion{static_cast<Uint16>(BaseVersion + 1)};
        static_assert(nextVersion < CurrentVersion);

        using Migration = ComponentMigration<Component, BaseVersion>;
        using ComponentAtBaseVersion = OldComponent<BaseVersion>;
        using ComponentAtNextVersion = OldComponent<nextVersion>;

        if constexpr (validMigration<Migration, ComponentAtBaseVersion,
                                     ComponentAtNextVersion>) {
            const ComponentAtBaseVersion* oldComponent{
                std::get_if<static_cast<std::size_t>(BaseVersion)>(
                    &migrationState)};
            if (oldComponent == nullptr) {
                return ComponentMigrationResult::MigrationFailed;
            }

            ComponentAtNextVersion nextComponent{};
            if (!(Migration::migrate(*oldComponent, nextComponent))) {
                return ComponentMigrationResult::MigrationFailed;
            }

            migrationState.template emplace<
                static_cast<std::size_t>(nextVersion)>(
                std::move(nextComponent));
            return ComponentMigrationResult::Success;
        }
        else {
            static_assert(validMigration<Migration, ComponentAtBaseVersion,
                                         ComponentAtNextVersion>,
                          "ComponentMigration::migrate() must accept the old "
                          "and next component types and return bool.");
            return ComponentMigrationResult::MigrationFailed;
        }
    }

    /**
     * Migrates from the last old version into the current component.
     */
    static ComponentMigrationResult migrateFinalStep(State& migrationState,
                                                     Component& currentComponent)
    {
        constexpr Uint16 baseVersion{static_cast<Uint16>(CurrentVersion - 1)};

        using Migration = ComponentMigration<Component, baseVersion>;
        using ComponentAtBaseVersion = OldComponent<baseVersion>;

        if constexpr (validMigration<Migration, ComponentAtBaseVersion,
                                     Component>) {
            const ComponentAtBaseVersion* oldComponent{
                std::get_if<static_cast<std::size_t>(baseVersion)>(
                    &migrationState)};
            if (oldComponent == nullptr) {
                return ComponentMigrationResult::MigrationFailed;
            }

            if (!(Migration::migrate(*oldComponent, currentComponent))) {
                return ComponentMigrationResult::MigrationFailed;
            }

            return ComponentMigrationResult::Success;
        }
        else {
            static_assert(validMigration<Migration, ComponentAtBaseVersion,
                                         Component>,
                          "ComponentMigration::migrate() must accept the old "
                          "and new component types and return bool.");
            return ComponentMigrationResult::MigrationFailed;
        }
    }

    template<std::size_t... Indices>
    static constexpr std::array<DeserializationFunction, VERSION_COUNT>
        makeDeserializationFunctions(std::index_sequence<Indices...>)
    {
        return {{
            &deserializeVersion<static_cast<Uint16>(Indices)>...,
        }};
    }

    static const std::array<DeserializationFunction, VERSION_COUNT>&
        deserializationFunctions()
    {
        static constexpr std::array<DeserializationFunction, VERSION_COUNT>
            functions{makeDeserializationFunctions(
                std::make_index_sequence<VERSION_COUNT>{})};
        return functions;
    }

    template<std::size_t... Indices>
    static constexpr std::array<IntermediateMigrationFunction,
                                INTERMEDIATE_MIGRATION_COUNT>
        makeIntermediateMigrationFunctions(std::index_sequence<Indices...>)
    {
        return {{
            &migrateIntermediateStep<static_cast<Uint16>(Indices)>...,
        }};
    }

    static const std::array<IntermediateMigrationFunction,
                            INTERMEDIATE_MIGRATION_COUNT>&
        intermediateMigrationFunctions()
    {
        static constexpr std::array<IntermediateMigrationFunction,
                                    INTERMEDIATE_MIGRATION_COUNT>
            functions{makeIntermediateMigrationFunctions(
                std::make_index_sequence<INTERMEDIATE_MIGRATION_COUNT>{})};
        return functions;
    }
};

/**
 * Migrates serializedComponent from serializedVersion to CurrentVersion.
 *
 * Every intermediate ComponentMigration specialization is applied in order.
 */
template<typename Component, Uint16 CurrentVersion>
ComponentMigrationResult
    migrateComponent(Uint16 serializedVersion,
                     std::span<const Uint8> serializedComponent,
                     Component& currentComponent)
{
    return ComponentMigrator<Component, CurrentVersion>::migrate(
        serializedVersion, serializedComponent, currentComponent);
}

} // namespace Server
} // namespace AM
