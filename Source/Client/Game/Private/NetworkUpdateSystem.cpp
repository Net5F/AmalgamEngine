#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageTools.h"
#include "ClientInput.h"
#include "Input.h"
#include "PlayerState.h"
#include "Peer.h"
#include "Log.h"
#include "Ignore.h"
#include "entt/entity/registry.hpp"
#include <memory>

namespace AM
{
namespace Client
{
NetworkUpdateSystem::NetworkUpdateSystem(Game& inGame, World& inWorld,
                                         Network& inNetwork)
: game(inGame)
, world(inWorld)
, network(inNetwork)
{
}

void NetworkUpdateSystem::sendInputState()
{
    if (RUN_OFFLINE) {
        // No need to send messages if we're running offline.
        return;
    }

    /* Send the updated state to the server. */
    // Only send new data if we've changed.
    Input& input = world.registry.get<Input>(world.playerEntity);
    if (input.isDirty) {
        // Get the current input state.
        ClientInput clientInput{game.getCurrentTick(), input};

        // Serialize the client inputs message.
        BinaryBufferSharedPtr messageBuffer
            = std::make_shared<BinaryBuffer>(Peer::MAX_MESSAGE_SIZE);
        unsigned int startIndex = CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE;
        std::size_t messageSize
            = MessageTools::serialize(*messageBuffer, clientInput, startIndex);

        // Fill the buffer with the appropriate message header.
        MessageTools::fillMessageHeader(MessageType::ClientInputs, messageSize,
                                        messageBuffer, CLIENT_HEADER_SIZE);

        // Send the message.
        network.send(messageBuffer);

        input.isDirty = false;
    }
}

} // namespace Client
} // namespace AM
