#ifndef NETWORKUPDATESYSTEM_H
#define NETWORKUPDATESYSTEM_H

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
 * This class is in charge of checking for data that needs to be sent, wrapping it
 * appropriately, and passing it to the Network's send queue.
 */
class NetworkUpdateSystem
{
public:
    NetworkUpdateSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Updates all connected clients with relevant world state.
     */
    void sendClientUpdates();

private:
    /**
     * Fills the given vector with the entities that must be sent to the given entityID
     * on this tick.
     */
    void constructAndSendUpdate(EntityID entityID, std::vector<EntityID>& dirtyEntities);

    /**
     * Serializes the given entity's relevant world
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

#endif /* NETWORKUPDATESYSTEM_H */
