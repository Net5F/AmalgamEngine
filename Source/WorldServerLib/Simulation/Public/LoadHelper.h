#pragma once

#include "entt/fwd.hpp"
#include <SDL3/SDL_stdinc.h>
#include <optional>
#include <span>
#include <vector>

namespace AM
{
namespace WorldServer
{
class World;
class Simulation;
class ItemData;
class GraphicData;

/**
 * Helper class for loading persisted world state.
 */
class LoadHelper
{
public:
    LoadHelper(World& inWorld, Simulation& inSimulation, ItemData& inItemData,
               const GraphicData& inGraphicData);

    /**
     * Loads saved non-client entities and adds them to the registry.
     *
     * If any entity fails to load, this is fatal (see loadEntity()).
     */
    void loadNonClientEntities();

    /**
     * Loads saved item definitions.
     *
     * If any item fails to load, this is fatal (a skipped item's IDs could be
     * reused, overwriting its saved data).
     */
    void loadItems();

    /**
     * Loads saved global stored values.
     *
     * If the map fails to load, this is fatal (the next save would overwrite
     * it).
     */
    void loadGlobalStoredValues();

    /**
     * A client entity's saved components for a single ClientPersistence.
     * Both spans are empty if the account had no saved data.
     */
    struct SerializedClientComponents {
        /** Ordered tagged engine component records. */
        std::span<const Uint8> engineComponents{};
        /** Ordered tagged project component records. */
        std::span<const Uint8> projectComponents{};
    };

    /**
     * Creates a client entity and loads its saved components onto it.
     *
     * Each set of saved components only has its matching components loaded
     * (e.g. if a component was changed from World to Account, its old World
     * data is ignored).
     *
     * If there's no saved Position (or the entity wouldn't be within the tile
     * map there), the entity is placed at the spawn point.
     *
     * If any saved component fails to parse, deserialize, or migrate, the
     * entity is destroyed and entt::null is returned. We don't spawn
     * partially-loaded entities, since the next save would overwrite the data
     * that failed to load.
     *
     * Note: This only loads saved components. The caller is responsible for
     *       adding defaults for any components that weren't loaded.
     *
     * @return The new entity, or entt::null if loading failed.
     */
    entt::entity
        loadClientEntity(Sint64 accountID,
                         const SerializedClientComponents& accountComponents,
                         const SerializedClientComponents& worldComponents);

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
     * Loads all valid engine components in serializedComponents that exist in
     * EntryList onto entity.
     *
     * @tparam EntryList A list of engine persisted component entries, e.g.
     *                   EnginePersistedComponentTypes::EntryTypes.
     * @return true if all components loaded successfully, else false.
     */
    template<typename EntryList>
    bool loadEngineComponents(
        entt::entity entity,
        const std::vector<SerializedComponent>& serializedComponents);

    /**
     * Loads all valid project components in serializedComponents that exist
     * in EntryList onto entity.
     *
     * @tparam EntryList A list of project persisted component entries, e.g.
     *                   ProjectPersistedComponentTypes::EntryTypes.
     * @return true if all components loaded successfully, else false.
     */
    template<typename EntryList>
    bool loadProjectComponents(
        entt::entity entity,
        const std::vector<SerializedComponent>& serializedComponents);

    /**
     * For any components in serializedComponents that exist in EntryList,
     * deserializes them and passes them to componentCallback.
     *
     * If a serialized component fails to deserialize, errors in Debug. In
     * Release, skips the component and proceeds.
     * If a component's ID isn't in EntryList, it's assumed to be deprecated
     * (or to belong to a different ClientPersistence) and is skipped.
     *
     * @param componentCallback A function of the form
     * bool(const auto& component) that expects the deserialized component,
     * and returns false if it failed to add it.
     * @return true if all components loaded successfully, else false.
     */
    template<typename EntryList, typename ComponentCallback>
    bool loadComponents(
        entt::entity entity,
        const std::vector<SerializedComponent>& serializedComponents,
        ComponentCallback&& componentCallback);

    /**
     * Deserializes serializedComponent as Entry::Component.
     *
     * If the serialized version is older than Entry::VERSION, applies each
     * registered migration step until the current version is reached.
     *
     * If the version is newer, a migration is missing or fails, or the data
     * fails to deserialize, errors in Debug. In Release, returns false so the
     * component can be skipped.
     */
    template<typename Entry>
    bool deserializeComponent(entt::entity entity,
                              const SerializedComponent& serializedComponent,
                              typename Entry::Component& component);

    /**
     * Initializes components with lazy-updated timers.
     */
    void initTimerComponents(entt::entity entity);

    /**
     * Returns the component with the given type ID, or nullptr if
     * serializedComponents doesn't contain it.
     */
    const SerializedComponent* findComponent(
        const std::vector<SerializedComponent>& serializedComponents,
        Uint16 typeID) const;

    /**
     * If the given engine component was saved in its ClientPersistence's set,
     * deserializes it into component.
     *
     * @return false if the component failed to deserialize, else true (even
     *         if it wasn't saved).
     */
    template<typename Component>
    bool getSavedClientComponent(
        const std::vector<SerializedComponent>& accountEngineComponents,
        const std::vector<SerializedComponent>& worldEngineComponents,
        std::optional<Component>& component);

    World& world;
    Simulation& simulation;
    ItemData& itemData;
    const GraphicData& graphicData;
};

} // namespace WorldServer
} // namespace AM
