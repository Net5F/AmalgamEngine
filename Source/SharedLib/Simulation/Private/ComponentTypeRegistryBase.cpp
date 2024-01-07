#include "ComponentTypeRegistryBase.h"

namespace AM
{

ComponentTypeRegistryBase::ComponentTypeRegistryBase(entt::registry& inRegistry)
: registry{inRegistry}
{
}

bool ComponentTypeRegistryBase::loadComponent(const SerializedComponent& component,
                                          entt::entity entity)
{
    // If the type isn't registered, return false.
    if (component.typeIndex >= loadFunctions.size()
        || !loadFunctions[component.typeIndex]) {
        return false;
    }

    return loadFunctions[component.typeIndex](entity, component, registry);
}

bool ComponentTypeRegistryBase::storeComponent(
    entt::entity entity, Uint8 typeIndex,
    std::vector<SerializedComponent>& destVec)
{
    // If the type isn't registered, return false.
    if (typeIndex >= storeFunctions.size() || !storeFunctions[typeIndex]) {
        return false;
    }

    return storeFunctions[typeIndex](entity, destVec, registry);
}

} // End namespace AM
