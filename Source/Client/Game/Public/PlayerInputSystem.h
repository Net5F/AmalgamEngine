#ifndef PLAYERINPUTSYSTEM_H
#define PLAYERINPUTSYSTEM_H

#include "GameDefs.h"
#include "Network.h"
#include "InputComponent.h"
#include "Message_generated.h"
#include "SDL_Events.h"

namespace AM
{
namespace Client
{

class Game;
class World;
class Network;

class PlayerInputSystem
{
public:
    PlayerInputSystem(Game& inGame, World& inWorld);

    /**
     * Updates the player's input state with the given event.
     */
    void processInputEvent(SDL_Event& event);

    /**
     * Adds the current player input state to the world's playerInputHistory.
     */
    void addCurrentInputsToHistory(Uint32 tickNum);

private:
    Game& game;
    World& world;
};

} // namespace Client
} // namespace AM

#endif /* PLAYERINPUTSYSTEM_H */
