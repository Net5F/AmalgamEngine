#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "SharedDefs.h"
#include "InputComponent.h"
#include "Message_generated.h"
#include <array>

namespace AM
{

class World;
class NetworkServer;

class MovementSystem
{
public:
    MovementSystem(World& inWorld, NetworkServer& inNetwork);

    /**
     * Updates movement components based on input state, moves position components
     * based on movement, updates sprites based on position.
     */
    void processMovements(double deltaMs);

private:
    /**
     * Updates the entity's velocity based on the current input states and delta time.
     */
    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates,
    double deltaMs);

    /**
     * Sends the given entity's relevant state information to all connected clients.
     */
    void broadcastEntity(EntityID entityID);

    /**
     * Converts AM input states to the flatbuffer equivalent.
     */
    fb::InputState convertToFbInputState(Input::State state);

    World& world;
    NetworkServer& network;

    flatbuffers::FlatBufferBuilder builder;
};

} // namespace AM

#endif /* MOVEMENTSYSTEM_H */
