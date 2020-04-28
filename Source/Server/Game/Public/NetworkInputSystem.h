#ifndef NETWORKINPUTSYSTEM_H
#define NETWORKINPUTSYSTEM_H

#include "Message_generated.h"
#include "InputComponent.h"

namespace AM
{
namespace Server
{

class World;
class Network;

class NetworkInputSystem
{
public:
    NetworkInputSystem(World& inWorld, Network& inNetwork);

    /**
     * Processes incoming EntityUpdate messages.
     */
    void processInputEvents();

private:
    World& world;
    Network& network;
};

} // namespace Server
} // namespace AM

#endif /* NETWORKINPUTSYSTEM_H */
