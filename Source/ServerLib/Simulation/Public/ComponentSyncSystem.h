#pragma once

#include "ComponentUpdate.h"
#include "EnttObserver.h"
#include <unordered_map>

namespace AM
{
namespace Server
{
struct SimulationContext;
class Simulation;
class World;
class Network;
class GraphicData;

/**
 * This system is the backing logic for the observed component type lists, 
 * e.g. EngineObserveBroadcastComponentTypes.h.
 *
 * When an observed component is updated, sends an update message to the 
 * appropriate clients.
 */
class ComponentSyncSystem
{
public:
    ComponentSyncSystem(const SimulationContext& inSimContext);

    /**
     * Sends updates for any observed components that were modified.
     */
    void sendUpdates();

private:
    void sendSelfUpdates();

    void sendInRangeUpdates();

    /**
     * Adds constructed or destroyed components from the given list to 
     * componentUpdateMap.
     */
    template <typename ComponentTypeList>
    void addConstructDestroyComponents(auto& constructObservers,
                                       auto& destroyObservers);

    /**
     * Adds updated components from the given list to * componentUpdateMap.
     */
    template <typename ComponentTypeList>
    void addUpdateComponents(auto& updateObservers);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used for fetching component data. */
    World& world;
    /** Used for sending updates to clients. */
    Network& network;
    /** Used to update Collision components when GraphicState is updated. */
    GraphicData& graphicData;

    // Note: Check the top of the cpp file for file-local types and variables.
    //       We keep some templated code there to reduce compile times.

    // Note: To optimize, we could store indices into a vector<ComponentUpdate>.
    //       Then we could re-use the vectors instead of re-allocating.
    /** Maps entityID -> a ComponentUpdate message containing that entity's
        data. We iterate the observers to detect changes, so this map lets us
        iteratively build the update messages component-by-component. */
    std::unordered_map<entt::entity, ComponentUpdate> componentUpdateMap;
};

} // namespace Server
} // namespace AM
