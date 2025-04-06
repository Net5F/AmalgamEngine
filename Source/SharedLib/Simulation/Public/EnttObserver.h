#pragma once

#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"

namespace AM
{
/**
 * A reactive storage that can observe changes to a single component type.
 *
 * To bind:
 *   observer.bind(registry);
 *   observer.on_construct<MyComponent>(); // on_update, on_destroy
 *
 * To view:
 *  for (entt::entity entity : observer) {
 *      // (process things here)
 *  }
 *  observer.clear();
 */
using EnttObserver = entt::reactive_mixin<entt::storage<void>>;

} // End namespace AM
