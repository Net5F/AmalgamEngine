#pragma once

#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"

namespace AM
{

namespace EnttObserver
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
using OneComponent = entt::reactive_mixin<entt::storage<void>>;

/**
 * A reactive storage that can observe changes to two component types.
 *
 * To bind:
 *   observer.bind(registry);
 *   observer.on_construct<MyComponent, &EnttObserver::twoComponentCallback>()
 *           .on_construct<MyComponent2, &EnttObserver::twoComponentCallback>();
 *
 * To view:
 *  for (auto [entity, hasBothComponents] : observer.each()) {
 *      if (!hasBothComponents) {
 *          continue;
 *      }
 *      
 *      // (process things here)
 * 
 *      observer.remove(entity);
 *  }
 */
using TwoComponent = entt::reactive_mixin<entt::storage<bool>>;

/**
 * Tracks if two components have been added to an entity in the given storage.
 */
static void
    twoComponentCallback(entt::reactive_mixin<entt::storage<bool>>& storage,
                         const entt::registry&, const entt::entity entity)
{
    // Add the entity to the storage on the first call, set the bool to true 
    // on the second call.
    storage.contains(entity) ? (storage.get(entity) = true)
                             : storage.emplace(entity, false);
}

} // End namespace EnttObserver

} // End namespace AM
