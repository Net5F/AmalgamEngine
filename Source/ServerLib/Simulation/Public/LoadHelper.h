#pragma once

#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <span>
#include <vector>

namespace AM
{
namespace Server
{
class World;
class Simulation;
class ItemData;

/**
 * Helper class for loading persisted world state.
 */
class LoadHelper
{
public:
    LoadHelper(World& inWorld, Simulation& inSimulation, ItemData& inItemData);

    /**
     * Loads saved non-client entities and adds them to the registry.
     */
    void loadNonClientEntities();

    /**
     * Loads saved item definitions.
     */
    void loadItems();

    /**
     * Loads saved entity stored value IDs and global stored values.
     */
    void loadStoredValues();

private:
    struct SerializedComponent {
        Uint16 typeID;
        Uint16 version;
        std::span<const Uint8> payload;
    };

    void loadEntity(entt::entity entity,
                    std::span<const Uint8> serializedEngineComponents,
                    std::span<const Uint8> serializedProjectComponents);

    /**
     * Parses the serializedComponents buffer, building up a list of each 
     * component's ID, version, and position within the buffer.
     */
    bool parseComponents(
        std::span<const Uint8> serializedComponents,
        std::vector<SerializedComponent>& components, entt::entity entity,
        const char* componentSetName) const;

    /**
     * Loads all valid engine components in serializedComponents onto entity.
     */
    void loadEngineComponents(
        entt::entity entity,
        const std::vector<SerializedComponent>& serializedComponents);

    /**
     * Loads all valid project components in serializedComponents onto entity.
     */
    void loadProjectComponents(
        entt::entity entity,
        const std::vector<SerializedComponent>& serializedComponents);

    /**
     * For any components in serializedComponents that exist in ComponentList, 
     * deserializes them and passes them to componentCallback.
     *
     * If a serialized component fails to deserialize, errors in Debug. In 
     * Release, skips the component and proceeds.
     * If a component's ID is unknown, it's assumed to be deprecated and is 
     * skipped.
     *
     * @param componentCallback A function of the form 
     * void(const auto& component) that expects the deserialized component.
     */
    template<typename ComponentList, typename ComponentCallback>
    void loadComponents(
        entt::entity entity,
        const std::vector<SerializedComponent>& serializedComponents,
        ComponentCallback&& componentCallback);

    /**
     * Deserializes serializedComponent as Entry::Component.
     *
     * If the version is outdated or the data fails to deserialize, errors in
     * Debug. In Release, returns false so the component can be skipped.
     */
    template<typename Entry>
    bool deserializeComponent(entt::entity entity,
                              const SerializedComponent& serializedComponent,
                              typename Entry::Component& component);

    /**
     * Initializes components with lazy-updated timers.
     */
    void initTimerComponents(entt::entity entity);

    World& world;
    Simulation& simulation;
    ItemData& itemData;
};

} // namespace Server
} // namespace AM
