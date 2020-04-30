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
