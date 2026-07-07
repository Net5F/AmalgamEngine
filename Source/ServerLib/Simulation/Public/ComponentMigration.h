#pragma once

#include "Deserialize.h"
#include <SDL3/SDL_stdinc.h>
#include <concepts>
#include <span>
#include <type_traits>

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
 * This class holds the component migration logic.
 */
template<typename Component, Uint16 CurrentVersion>
class ComponentMigrator
{
public:
    /**
     * Migrates from serializedVersion to CurrentVersion.
     */
    static ComponentMigrationResult
        migrate(Uint16 serializedVersion,
                std::span<const Uint8> serializedComponent,
                Component& currentComponent)
    {
        static_assert(CurrentVersion > 0);
        return dispatchMigration<0>(serializedVersion, serializedComponent,
                                    currentComponent);
    }

private:
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

    /**
     * Runs the migration chain, from BaseVersion to CurrentVersion.
     */
    template<Uint16 BaseVersion, typename OldComponent>
    static ComponentMigrationResult
        runMigrationChain(const OldComponent& oldComponent,
                          Component& currentComponent)
    {
        static_assert(BaseVersion < CurrentVersion);

        if constexpr (!hasMigration<BaseVersion>) {
            static_assert(hasMigration<BaseVersion>,
                          "Missing ComponentMigration specialization.");
            return ComponentMigrationResult::MigrationFailed;
        }
        else {
            using Migration = ComponentMigration<Component, BaseVersion>;
            static_assert(
                std::is_same_v<OldComponent, typename Migration::OldComponent>,
                "A component migration received the wrong old component type.");

            if constexpr ((BaseVersion + 1) == CurrentVersion) {
                if constexpr (validMigration<Migration, OldComponent,
                                             Component>) {
                    if (!(Migration::migrate(oldComponent, currentComponent))) {
                        return ComponentMigrationResult::MigrationFailed;
                    }
                    return ComponentMigrationResult::Success;
                }
                else {
                    static_assert(
                        validMigration<Migration, OldComponent, Component>,
                        "ComponentMigration::migrate() must accept the old and "
                        "new component types and return bool.");
                    return ComponentMigrationResult::MigrationFailed;
                }
            }
            else {
                constexpr Uint16 nextVersion{
                    static_cast<Uint16>(BaseVersion + 1)};

                if constexpr (!hasMigration<nextVersion>) {
                    static_assert(hasMigration<nextVersion>,
                                  "Missing intermediate ComponentMigration "
                                  "specialization.");
                    return ComponentMigrationResult::MigrationFailed;
                }
                else {
                    using NextComponent =
                        typename ComponentMigration<Component,
                                                    nextVersion>::OldComponent;
                    NextComponent nextComponent{};

                    if constexpr (validMigration<Migration, OldComponent,
                                                 NextComponent>) {
                        if (!(Migration::migrate(oldComponent,
                                                 nextComponent))) {
                            return ComponentMigrationResult::MigrationFailed;
                        }
                        return runMigrationChain<nextVersion>(nextComponent,
                                                              currentComponent);
                    }
                    else {
                        static_assert(
                            validMigration<Migration, OldComponent,
                                           NextComponent>,
                            "ComponentMigration::migrate() must accept the old "
                            "and next component types and return bool.");
                        return ComponentMigrationResult::MigrationFailed;
                    }
                }
            }
        }
    }

    /**
     * Deserializes the component and begins the migration chain.
     */
    template<Uint16 BaseVersion>
    static ComponentMigrationResult
        migrateFromVersion(std::span<const Uint8> serializedComponent,
                           Component& currentComponent)
    {
        if constexpr (!hasMigration<BaseVersion>) {
            static_assert(hasMigration<BaseVersion>,
                          "Missing ComponentMigration specialization.");
            return ComponentMigrationResult::MigrationFailed;
        }
        else {
            using OldComponent =
                typename ComponentMigration<Component,
                                            BaseVersion>::OldComponent;
            OldComponent oldComponent{};
            if (!Deserialize::fromBuffer(serializedComponent.data(),
                                         serializedComponent.size(),
                                         oldComponent)) {
                return ComponentMigrationResult::DeserializationFailed;
            }

            return runMigrationChain<BaseVersion>(oldComponent,
                                                  currentComponent);
        }
    }

    /**
     * Recursively increments CandidateVersion until serializedVersion is 
     * reached.
     * Used to move serializedVersion into template context.
     */
    template<Uint16 CandidateVersion>
    static ComponentMigrationResult
        dispatchMigration(Uint16 serializedVersion,
                          std::span<const Uint8> serializedComponent,
                          Component& currentComponent)
    {
        if (serializedVersion == CandidateVersion) {
            return migrateFromVersion<CandidateVersion>(serializedComponent,
                                                        currentComponent);
        }

        if constexpr ((CandidateVersion + 1) < CurrentVersion) {
            return dispatchMigration<static_cast<Uint16>(CandidateVersion + 1)>(
                serializedVersion, serializedComponent, currentComponent);
        }
        else {
            return ComponentMigrationResult::UnsupportedVersion;
        }
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
