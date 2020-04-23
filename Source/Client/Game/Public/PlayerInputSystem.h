#ifndef PLAYERINPUTSYSTEM_H
#define PLAYERINPUTSYSTEM_H

#include "SharedDefs.h"
#include "NetworkClient.h"
#include "InputComponent.h"
#include "Message_generated.h"

namespace AM
{

class World;
class Network;

class PlayerInputSystem
{
public:
    PlayerInputSystem(World& inWorld, NetworkClient& inNetwork);

    Input processInputEvents();

    void sendInputState();

private:
    static constexpr int BUILDER_BUFFER_SIZE = 512;

    /**
     * Converts AM input states to the flatbuffer equivalent.
     */
    fb::InputState convertToFbInputState(Input::State state);

    World& world;
    NetworkClient& network;
    flatbuffers::FlatBufferBuilder builder;
};

} // namespace AM

#endif /* PLAYERINPUTSYSTEM_H */
