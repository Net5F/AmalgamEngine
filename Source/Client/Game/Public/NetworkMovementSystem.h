#ifndef NETWORKMOVEMENTSYSTEM_H
#define NETWORKMOVEMENTSYSTEM_H

#include "SharedDefs.h"
#include "InputComponent.h"
#include "Message_generated.h"
#include <array>
#include <vector>

namespace AM
{
namespace Client
{

class Game;
class World;
class Network;

class NetworkMovementSystem
{
public:
    NetworkMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Updates movement components based on input state, moves position components based on movement, updates sprites based on position.
     */
    void processServerMovements();

private:
    static constexpr int MIN_BUFFERED_MESSAGES = 2;

    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates);

    // TODO: Can replace with a ring buffer if this ever is an issue.
    std::vector<BinaryBufferPtr> stateMessageQueue;

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM

#endif /* NETWORKMOVEMENTSYSTEM_H */
