#pragma once

#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "boost/mp11/list.hpp"
#include "bitsery/traits/vector.h"
#include "bitsery/ext/std_variant.h"
#include <variant>
#include <vector>

namespace AM
{
// Note: Persisted components are server-only.
namespace Server
{
/**
 * A variant that holds a persisted engine component.
 *
 * Used by the server to save entity state to the database.
 */
using EnginePersistedComponent
    = boost::mp11::mp_rename<EnginePersistedComponentTypes, std::variant>;

template<typename S>
void serialize(S& serializer,
               std::vector<EnginePersistedComponent>& engineComponents)
{
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(
            engineComponents,
            boost::mp11::mp_size<EnginePersistedComponentTypes>::value,
            [](typename S::BPEnabledType& serializer,
               EnginePersistedComponent& component) {
                serializer.ext(component, bitsery::ext::StdVariant{});
            });
    });
}

/**
 * A variant that holds a persisted engine component.
 *
 * Used by the server to save entity state to the database.
 */
using ProjectPersistedComponent
    = boost::mp11::mp_rename<ProjectPersistedComponentTypes, std::variant>;

template<typename S>
void serialize(S& serializer,
               std::vector<ProjectPersistedComponent>& projectComponents)
{
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(
            projectComponents,
            boost::mp11::mp_size<ProjectPersistedComponentTypes>::value,
            [](typename S::BPEnabledType& serializer,
               ProjectPersistedComponent& component) {
                serializer.ext(component, bitsery::ext::StdVariant{});
            });
    });
}

} // End namespace Server
} // End namespace AM
