#ifndef PLAYERINPUTSYSTEM_H
#define PLAYERINPUTSYSTEM_H

#include "SharedDefs.h"
#include "InputComponent.h"
#include "Message_generated.h"

namespace AM
{

class World;
class Network;

class PlayerInputSystem
{
public:
    PlayerInputSystem(World& inWorld, Network& inNetwork);

    Input processInputEvents();

    void sendInputState();

private:
    static constexpr int BUILDER_BUFFER_SIZE = 512;

    fb::InputState convertToFbInputState(Input::State state);

    World& world;
    Network& network;
    flatbuffers::FlatBufferBuilder builder;
};

} // namespace AM

#endif /* PLAYERINPUTSYSTEM_H */
