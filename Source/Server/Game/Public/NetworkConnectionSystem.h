#ifndef NETWORKCONNECTIONSYSTEM_H_
#define NETWORKCONNECTIONSYSTEM_H_

#include "Message_generated.h"
#include "NetworkDefs.h"
#include "GameDefs.h"

namespace AM
{
namespace Server
{

class Game;
class World;
class Network;

/**
 * This class is in charge of processing connect/disconnect events and
 * updating the associated client entities.
 */
class NetworkConnectionSystem
{
public:
    NetworkConnectionSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /** Processes the effects of new connections and disconnects on the sim. */
    void processConnectionEvents();

private:
    /**
     * Processes all newly connected clients, adding them to the sim.
     */
    void processConnectEvents();

    /**
     * Processes all newly disconnected clients, removing them from the sim.
     */
    void processDisconnectEvents();

    /**
     * Sends a connection response to the client with the given networkID.
     *
     * @param networkID  The client's network ID to send the connection response to.
     * @param newEntityID  The entity ID that the Sim associated with the client.
     */
    void sendConnectionResponse(NetworkID networkID, EntityID newEntityID,
                                float spawnX, float spawnY);

    Game& game;
    World& world;
    Network& network;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;
};

} // End namespace Server
} // End namespace AM

#endif /* End NETWORKCONNECTIONSYSTEM_H_ */
