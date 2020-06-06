#ifndef NETWORKOUTPUTSYSTEM_H
#define NETWORKOUTPUTSYSTEM_H

#include "Message_generated.h"
#include "GameDefs.h"

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
     * Updates all connected clients with relevant world state.
     */
    void sendClientUpdates();

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
