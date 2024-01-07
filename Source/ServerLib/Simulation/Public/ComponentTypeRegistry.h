#pragma once

#include "ComponentTypeRegistryBase.h"
#include "ReplicatedComponentList.h"
#include "entt/entity/observer.hpp"

namespace AM
{
namespace Server
{

/**
 * See ComponentTypeRegistryBase class comment.
 *
 * Beyond the base functionality, this class supports observed components, 
 * persisted components, and AI components.
 */
class ComponentTypeRegistry : public ComponentTypeRegistryBase {
public:
    ComponentTypeRegistry(entt::registry& inRegistry);

    /**
     * Used to register client-relevant components.
     *
     * When a client comes in range of an entity, an Init message that includes
     * these components will be sent (if the entity possesses any of them).
     *
     * If you want a component to additionally be sent whenever it's updated, 
     * add it as an observed component.
     */
    template<typename ReplicatedComponentTypes>
    void registerReplicatedComponents();

    /**
     * Used to register observed components.
     *
     * When an observed component is updated (using patch() or replace()), an 
     * Update message containing the component will be sent by the server to 
     * all nearby clients.
     *
     * Note: Only the server uses observed components.
     */
    template<typename ReplicatedComponentTypes, typename ObservedComponentTypes>
    void registerObservedComponents();

    //template<typename ComponentTypeList>
    //void registerPersistedComponents();

    struct ObservedComponent
    {
        Uint8 typeIndex{};
        entt::observer observer{};
    };
    /** Holds the observers for each registered observed component. */
    std::vector<ObservedComponent> observedComponents;

private:
    /**
     * Adds the index of T to entity's ReplicatedComponentList component.
     */
    template<typename T, Uint8 typeIndex>
    void onComponentConstructed(entt::registry& registry, entt::entity entity);

    /**
     * Removes the index of T from entity's ReplicatedComponentList component.
     */
    template<typename T, Uint8 typeIndex>
    void onComponentDestroyed(entt::registry& registry, entt::entity entity);
};

template<typename ReplicatedComponentTypes>
void ComponentTypeRegistry::registerReplicatedComponents()
{
    // Add the store and load functions.
    ComponentTypeRegistryBase::registerReplicatedComponents<
        ReplicatedComponentTypes>();

    // For each given component type.
    boost::mp11::mp_for_each<ReplicatedComponentTypes>([&](auto I) {
        using Component = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes, Component>::value};

        // Add callbacks to maintain the entity's ReplicatedComponentsList 
        // when this component is constructed or destroyed.
        registry.on_construct<Component>()
            .template connect<&ComponentTypeRegistry::onComponentConstructed<
                Component, static_cast<Uint8>(typeIndex)>>(*this);
        registry.on_destroy<Component>()
            .template connect<&ComponentTypeRegistry::onComponentDestroyed<
                Component, static_cast<Uint8>(typeIndex)>>(*this);
    });
}

template<typename ReplicatedComponentTypes, typename ObservedComponentTypes>
void ComponentTypeRegistry::registerObservedComponents()
{
    // entt::observer can't be copied or moved, so we have to fill a temp 
    // vector and move it at the end.
    std::vector<ObservedComponent> tempVec(
        boost::mp11::mp_size<ObservedComponentTypes>::value);

    // For each given component type.
    std::size_t observedComponentIndex{0};
    boost::mp11::mp_for_each<ObservedComponentTypes>([&](auto I) {
        using Component = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes, Component>::value};
        static_assert(
            typeIndex != boost::mp11::mp_size<ReplicatedComponentTypes>::value,
            "Observed components must also be present in "
            "ReplicatedComponentTypes.");

        // Register an observer for this component type.
        // TODO: If a client is near an entity when it's constructed, it'll
        //       receive both an EntityInit and a ComponentUpdate (from the
        //       group observer). It'd be nice if we could find a way to just
        //       send one, but until then it isn't a huge cost.
        auto& observedComponent{tempVec[observedComponentIndex]};
        observedComponent.typeIndex = static_cast<Uint8>(typeIndex);
        observedComponent.observer.connect(
            registry,
            entt::collector.group<Component>().template update<Component>());

        observedComponentIndex++;
    });

    observedComponents = std::move(tempVec);
}

//template<typename ComponentTypeList>
//void ComponentTypeRegistryBase::registerPersistedComponents()
//{
//    // For each type in the list, register a different store and load 
//    // function? When we need to store/load an entity, call every one and 
//    // ignore failure?
//}

template<typename T, Uint8 typeIndex>
void ComponentTypeRegistry::onComponentConstructed(entt::registry& registry,
                                                   entt::entity entity)
{
    // Add the component to the entity's tracking vector.
    auto& replicatedComponents{
        registry.get_or_emplace<ReplicatedComponentList>(entity)};
    replicatedComponents.typeIndices.push_back(typeIndex);
}

template<typename T, Uint8 typeIndex>
void ComponentTypeRegistry::onComponentDestroyed(entt::registry& registry,
                                                 entt::entity entity)
{
    // If the component is in the entity's tracking vector, remove it.
    if (auto replicatedComponents
        = registry.try_get<ReplicatedComponentList>(entity)) {
        std::erase(replicatedComponents->typeIndices, typeIndex);
    }
}

} // End namespace Server
} // End namespace AM
