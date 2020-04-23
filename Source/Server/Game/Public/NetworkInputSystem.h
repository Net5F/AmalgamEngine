#ifndef NETWORKINPUTSYSTEM_H
#define NETWORKINPUTSYSTEM_H

#include "Message_generated.h"
#include "InputComponent.h"

namespace AM
{

class World;
class NetworkServer;

class NetworkInputSystem
{
public:
    NetworkInputSystem(World& inWorld, NetworkServer& inNetwork);

    /**
     * Processes incoming EntityUpdate messages.
     */
    void processInputEvents();

private:
    /**
     * Converts flatbuffer input states to the AM equivalent.
     */
    Input::State convertToAMInputState(fb::InputState state);

    World& world;
    NetworkServer& network;
};

} // namespace AM

#endif /* NETWORKINPUTSYSTEM_H */
