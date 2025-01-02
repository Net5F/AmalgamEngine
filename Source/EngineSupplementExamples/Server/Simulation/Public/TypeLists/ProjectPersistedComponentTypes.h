#pragma once

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
 * The version number of the project's components and component list.
 *
 * If ProjectPersistedComponentTypes is changed in any way, or the fields of any
 * component in the list are changed in a way that changes their serialization, 
 * you must increment this number and run a migration.
 */
static constexpr unsigned int PROJECT_COMPONENTS_VERSION{0};

/**
 * All of the project's component types that should be saved to the database 
 * and loaded at startup.
 *
 * Note: If you change this list in any way, or change the fields of any included
 *       types in a way that breaks serialization, you must increment 
 *       PROJECT_COMPONENTS_VERSION and run a migration.
 */
using ProjectPersistedComponentTypes = boost::mp11::mp_list<>;

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

#endif // defined(AM_OVERRIDE_CONFIG)
