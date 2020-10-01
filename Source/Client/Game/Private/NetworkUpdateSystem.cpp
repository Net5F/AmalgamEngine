#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageTools.h"
#include "ClientInputs.h"
#include "Peer.h"
#include "Debug.h"
#include <memory>

namespace AM
{
namespace Client
{

NetworkUpdateSystem::NetworkUpdateSystem(Game& inGame, World& inWorld, Network& inNetwork)
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
    if (world.playerIsDirty) {
        // Only send new data if we've changed.
        EntityID playerID = world.playerID;
        ClientInputs clientInputs = {playerID, game.getCurrentTick(),
                world.inputs[playerID]};

        // Serialize the client inputs message.
        BinaryBufferSharedPtr messageBuffer = std::make_shared<BinaryBuffer>(
            Peer::MAX_MESSAGE_SIZE);
        std::size_t messageSize = MessageTools::serialize(*messageBuffer, clientInputs);

        // TEMP - Shift the elements of the vector to make room for the header.
        // TODO: Replace this with serializing straight into the proper spot.
        int numToShift = CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE;
        messageBuffer->insert(messageBuffer->begin(), numToShift, 0);

        // Fill the buffer with the appropriate message header.
        Network::fillMessageHeader(MessageType::ClientInputs, messageSize, messageBuffer);

        // Send the message.
        network.send(messageBuffer);
    }
    // TODO: Heartbeat from the network tick if no messages have been sent.

    world.playerIsDirty = false;
}

} // namespace Client
} // namespace AM
