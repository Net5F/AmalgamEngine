#include "NetworkUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "InputChangeRequest.h"
#include "Input.h"
#include "PlayerState.h"
#include "IsDirty.h"
#include "Peer.h"
#include "Config.h"
#include "Log.h"
#include "Ignore.h"
#include "entt/entity/registry.hpp"
#include <memory>

namespace AM
{
namespace Client
{
NetworkUpdateSystem::NetworkUpdateSystem(Simulation& inSim, World& inWorld,
                                         Network& inNetwork)
: sim(inSim)
, world(inWorld)
, network(inNetwork)
{
}

void NetworkUpdateSystem::sendInputState()
{
    if (Config::RUN_OFFLINE) {
        // No need to send messages if we're running offline.
        return;
    }

    /* Send the updated state to the server. */
    // Only send new data if we've changed.
    entt::registry& registry = world.registry;
    if (registry.all_of<IsDirty>(world.playerEntity)) {
        // Get the current input state.
        Input& input = registry.get<Input>(world.playerEntity);

        // Send the client input message.
        network.serializeAndSend<InputChangeRequest>({sim.getCurrentTick(), input});

        registry.remove<IsDirty>(world.playerEntity);
    }
}

} // namespace Client
} // namespace AM
