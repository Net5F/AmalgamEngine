#pragma once

#include "PersistedComponent.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include "boost/mp11/algorithm.hpp"
#include <vector>

namespace AM
{
namespace Server
{

/**
 * Used by the server to save entity state to the database.
 *
 * Contains the full persistent state for a single entity.
 */
struct PersistedEntityData {
    /** This entity's persisted components. */
    std::vector<PersistedComponent> components{};
};

template<typename S>
void serialize(S& serializer, PersistedEntityData& persistedEntityData)
{
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(persistedEntityData.components,
                      boost::mp11::mp_size<PersistedComponentTypes>::value,
                      [](typename S::BPEnabledType& serializer,
                         PersistedComponent& component) {
                          serializer.ext(component, bitsery::ext::StdVariant{});
                      });
    });
}

} // End namespace Server
} // End namespace AM
