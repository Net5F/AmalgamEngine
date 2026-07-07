#pragma once

#include "ComponentMigration.h"

namespace AM
{
namespace Server
{

/**
 * Component migration:
 *
 * When incrementing a component from version N to N + 1:
 * 1. Add a ComponentMigration<Component, N> specialization.
 * 2. Copy the old struct and serialize() function into the specialization.
 * 3. Rename the old struct to OldComponent.
 * 4. Move serialize() into OldComponent and change it to use the member-
 *    function signature (get rid of second parameter).
 * 5. Increment the component entry's version in
 *    EnginePersistedComponentTypes.h.
 *
 * When loading, migration chaining happens automatically (e.g. v0 -> v1 -> v2).
 * In migrate(), newComponent is the output of the migration step (either the 
 * new final component, or the next component in the migration chain).
 */

/*
 * Example:
 *
 * // ExampleEngineComponent v0 -> v1.
 * template<>
 * struct ComponentMigration<ExampleEngineComponent, 0> {
 *     struct OldComponent {
 *         Uint32 oldField{};
 *
 *         template<typename S>
 *         void serialize(S& serializer)
 *         {
 *             serializer.value4b(oldField);
 *         }
 *     };
 *
 *     static bool migrate(const OldComponent& oldComponent,
 *                         auto& newComponent)
 *     {
 *         newComponent.newField = oldComponent.oldField;
 *         return true;
 *     }
 * };
 */

} // namespace Server
} // namespace AM
