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
     * Updates movement components based on input state, moves position components based on movement, updates sprites based on position.
     */
    void processMovements();

private:
    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates);

    /**
     * Sends the given entity's relevant state information to all connected clients.
     */
    void broadcastEntity(EntityID entityID);

    World& world;
    NetworkServer& network;

    flatbuffers::FlatBufferBuilder builder;
};

} // namespace AM

#endif /* MOVEMENTSYSTEM_H */
