#ifndef NETWORKMOVEMENTSYSTEM_H
#define NETWORKMOVEMENTSYSTEM_H

#include "SharedDefs.h"
#include "InputComponent.h"
#include <array>

namespace AM
{

class World;
class NetworkClient;

class NetworkMovementSystem
{
public:
    NetworkMovementSystem(World& inWorld, NetworkClient& inNetwork);

    /**
     * Updates movement components based on input state, moves position components based on movement, updates sprites based on position.
     */
    void processServerMovements();

private:
    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates);

    World& world;
    NetworkClient& network;
};

} // namespace AM

#endif /* NETWORKMOVEMENTSYSTEM_H */
