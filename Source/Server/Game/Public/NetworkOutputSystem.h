#ifndef NETWORKOUTPUTSYSTEM_H
#define NETWORKOUTPUTSYSTEM_H

#include "Message_generated.h"
#include "SharedDefs.h"

namespace AM
{
namespace Server
{

class Game;
class World;
class Network;

/**
 *
 */
class NetworkOutputSystem
{
public:
    NetworkOutputSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * If there is dirty state information, sends it to all connected clients.
     * Else, sends an empty message as a heartbeat.
     */
    void broadcastDirtyEntities();

private:
    /**
     * Serializes the given entity's relevant world data.
     * @param entityID  The entity to serialize.
     * @return An offset where the data was stored in the builder.
     */
    flatbuffers::Offset<AM::fb::Entity> serializeEntity(EntityID entityID);

    Game& game;
    World& world;
    Network& network;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;
};

} // namespace Server
} // namespace AM

#endif /* NETWORKOUTPUTSYSTEM_H */
