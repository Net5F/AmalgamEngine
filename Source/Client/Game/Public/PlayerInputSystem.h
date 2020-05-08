#ifndef PLAYERINPUTSYSTEM_H
#define PLAYERINPUTSYSTEM_H

#include "SharedDefs.h"
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

    void processInputEvent(SDL_Event& event);

private:
    Game& game;
    World& world;
};

} // namespace Client
} // namespace AM

#endif /* PLAYERINPUTSYSTEM_H */
