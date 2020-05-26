#ifndef NPCMOVEMENTSYSTEM_H
#define NPCMOVEMENTSYSTEM_H

#include "GameDefs.h"
#include "NetworkDefs.h"
#include "InputComponent.h"
#include <array>

namespace AM
{
namespace Client
{

class Game;
class World;
class Network;

class NpcMovementSystem
{
public:
    NpcMovementSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Updates movement components based on input state, moves position components
     * based on movement, updates sprites based on position.
     */
    void processMovements(float deltaSeconds);

private:
    /**
     * Updates the entity's velocity based on the current input states and delta time.
     */
    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates,
    float deltaSeconds);

    // TODO: Can replace with a ring buffer if this ever is an issue.
    std::vector<BinaryBufferPtr> stateMessageQueue;

    Game& game;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM

#endif /* NPCMOVEMENTSYSTEM_H */
