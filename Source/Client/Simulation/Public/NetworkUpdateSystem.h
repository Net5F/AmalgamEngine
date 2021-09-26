#pragma once

#include <array>

namespace AM
{
namespace Client
{
class Simulation;
class World;
class Network;

/**
 * This class is in charge of checking for data that needs to be sent, wrapping
 * it appropriately, and passing it to the Network's send queue.
 */
class NetworkUpdateSystem
{
public:
    NetworkUpdateSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * If the player inputs have changed, sends them to the server.
     * Else, sends an empty message as a heartbeat.
     */
    void sendInputState();

private:
    /** Used to get the current tick. */
    Simulation& sim;
    /** Used to access components. */
    World& world;
    /** Used to send update messages. */
    Network& network;
};

} // namespace Client
} // namespace AM
