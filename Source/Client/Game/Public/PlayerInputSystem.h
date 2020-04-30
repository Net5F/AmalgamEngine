#ifndef PLAYERINPUTSYSTEM_H
#define PLAYERINPUTSYSTEM_H

#include "SharedDefs.h"
#include "Network.h"
#include "InputComponent.h"
#include "Message_generated.h"
#include "SDL_Events.h"

namespace AM
{
namespace Client
{

class Game;
class World;
class Network;

class PlayerInputSystem
{
public:
    PlayerInputSystem(Game& inGame, World& inWorld, Network& inNetwork);

    void processInputEvent(SDL_Event& event);

    void sendInputState();

private:
    /**
     * Serializes the given entity's relevant world data.
     * @param entityID  The entity to serialize.
     * @return An offset where the data was stored in the builder.
     */
    flatbuffers::Offset<AM::fb::Entity> serializeEntity(EntityID entityID);

    static constexpr int BUILDER_BUFFER_SIZE = 512;

    Game& game;
    World& world;
    Network& network;
    flatbuffers::FlatBufferBuilder builder;

    bool stateIsDirty;
};

} // namespace Client
} // namespace AM

#endif /* PLAYERINPUTSYSTEM_H */
