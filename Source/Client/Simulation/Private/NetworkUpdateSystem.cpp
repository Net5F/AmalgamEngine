#include "NetworkUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Serialize.h"
#include "MessageTools.h"
#include "ClientInput.h"
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
    if (registry.has<IsDirty>(world.playerEntity)) {
        // Get the current input state.
        Input& input = registry.get<Input>(world.playerEntity);
        ClientInput clientInput{sim.getCurrentTick(), input};

        // Serialize the client inputs message.
        BinaryBufferSharedPtr messageBuffer
            = std::make_shared<BinaryBuffer>(Peer::MAX_MESSAGE_SIZE);
        unsigned int startIndex = CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE;
        std::size_t messageSize
            = Serialize::toBuffer(*messageBuffer, clientInput, startIndex);

        // Fill the buffer with the appropriate message header.
        MessageTools::fillMessageHeader(MessageType::ClientInputs, messageSize,
                                        messageBuffer, CLIENT_HEADER_SIZE);

        // Send the message.
        network.send(messageBuffer);

        registry.remove<IsDirty>(world.playerEntity);
    }
}

} // namespace Client
} // namespace AM
