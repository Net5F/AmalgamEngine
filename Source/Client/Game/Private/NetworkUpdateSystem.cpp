#include "NetworkUpdateSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "ClientInputs.h"
#include "MessageTools.h"
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
//    if (world.playerIsDirty) {
        // Only send new data if we've changed.
        // TODO: Don't send a message if we haven't changed, let the network do the heartbeat.
        EntityID playerID = world.playerID;
        ClientInputs clientInputs = {playerID, game.getCurrentTick(),
                world.inputs[playerID]};

        // Serialize the client inputs message.
        BinaryBufferSharedPtr messageBuffer = std::make_shared<BinaryBuffer>(
            Peer::MAX_MESSAGE_SIZE);
        std::size_t writtenSize = MessageTools::serialize(*messageBuffer, clientInputs);

        // Shrink the buffer to fit and send the message.
        messageBuffer->resize(writtenSize);
        network.send(messageBuffer);
//    }

    world.playerIsDirty = false;
}

} // namespace Client
} // namespace AM
