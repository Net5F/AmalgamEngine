#pragma once

#include "SerializedComponent.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "Log.h"
#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <vector>

namespace AM
{

/**
 * A registry that provides a generic, type-erased interface for managing 
 * entity components.
 * 
 * Type erasure is necessary because the project may provide its own component 
 * types. Since these types aren't visible to the engine, we need some method 
 * of type erasure in order for the engine to manage them.
 */
class ComponentTypeRegistryBase {
public:
    ComponentTypeRegistryBase(entt::registry& inRegistry);

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
     * Deserializes the given component and adds it to entity.
     * If entity already has the given component type, replaces it.
     *
     * @return false if the component type isn't registered or failed to 
     *         deserialize, else true.
     */
    bool loadComponent(const SerializedComponent& component,
                       entt::entity entity);

    /**
     * Gets the component associated with typeIndex, serializes it, and pushes 
     * it into destVec.
     * 
     * @return false if the specified component type isn't registered or the 
     *         entity doesn't have it, else true.
     */
    bool storeComponent(entt::entity entity, Uint8 typeIndex,
                        std::vector<SerializedComponent>& destVec);

protected:
    /**
     * Adds a function for the given type to loadFunctions.
     */
    template<typename T>
    void addLoadComponentFunc();

    /**
     * Adds a function for the given type and index to storeFunctions.
     */
    template<typename T>
    void addStoreComponentFunc(Uint8 typeIndex);

    entt::registry& registry;

    using LoadComponentFunction = std::function<bool(
        entt::entity, const SerializedComponent&, entt::registry&)>;
    /** Holds functions for deserializing a component and pushing it into the 
        registry.
        The indices in this vector match the indices of the type list given 
        to registerReplicatedComponents(). */
    std::vector<LoadComponentFunction> loadFunctions;

    using StoreComponentFunction = std::function<bool(
        entt::entity, std::vector<SerializedComponent>&, entt::registry&)>;
    /** Holds functions for getting a component and serializing it into a 
        buffer.
        The indices in this vector match the indices of the type list given 
        to registerReplicatedComponents(). */
    std::vector<StoreComponentFunction> storeFunctions;
};

template<typename ReplicatedComponentTypes>
void ComponentTypeRegistryBase::registerReplicatedComponents()
{
    // For each given component type.
    boost::mp11::mp_for_each<ReplicatedComponentTypes>([&](auto I) {
        using Component = decltype(I);
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes,
                                 Component>::value};

        // Add load and store functions for this component type.
        addLoadComponentFunc<Component>();
        addStoreComponentFunc<Component>(typeIndex);
    });
}

template<typename T>
void ComponentTypeRegistryBase::addLoadComponentFunc()
{
    auto loadComponent = [](entt::entity entity,
                            const SerializedComponent& serializedComponent,
                            entt::registry& registry) {
        T component{};
        auto& buffer{serializedComponent.buffer};

        // Deserialize the component. If unsuccessful, return false.
        bool success{Deserialize::fromBuffer(buffer.data(), buffer.size(),
                                             component)};
        if (!success) {
            return false;
        }

        // Success. Add the component to the entity and return true.
        registry.emplace_or_replace<T>(entity, component);
        return true;
    };
    loadFunctions.emplace_back(std::move(loadComponent));
}

template<typename T>
void ComponentTypeRegistryBase::addStoreComponentFunc(Uint8 typeIndex)
{
    auto storeComponent = [typeIndex](entt::entity entity,
                                      std::vector<SerializedComponent>& destVec,
                                      entt::registry& registry) {
        if (registry.all_of<T>(entity)) {
            if constexpr (std::is_empty_v<T>) {
                // Note: Can't registry.get() empty types.
                destVec.emplace_back(typeIndex);
            }
            else {
                T& component{registry.get<T>(entity)};
                SerializedComponent& serializedComponent{
                    destVec.emplace_back(typeIndex)};

                // Serialize the component into the buffer.
                serializedComponent.buffer.resize(
                    Serialize::measureSize(component));
                Serialize::toBuffer(serializedComponent.buffer.data(),
                                    serializedComponent.buffer.size(),
                                    component);
            }

            return true;
        }

        return false;
    };
    storeFunctions.emplace_back(std::move(storeComponent));
}

} // End namespace AM
