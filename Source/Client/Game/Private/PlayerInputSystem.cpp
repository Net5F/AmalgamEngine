#include "PlayerInputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

PlayerInputSystem::PlayerInputSystem(Game& inGame, World& inWorld)
: game(inGame),
  world(inWorld)
{
}

void PlayerInputSystem::processInputEvent(SDL_Event& event)
{
    // Process all events.
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        Input::State inputState =
        (event.type == SDL_KEYDOWN) ? Input::Pressed : Input::Released;
        Input keyInput = { Input::None, inputState };

        switch (event.key.keysym.sym)
        {
            case SDLK_w:
                keyInput.type = Input::Up;
                break;
            case SDLK_a:
                keyInput.type = Input::Left;
                break;
            case SDLK_s:
                keyInput.type = Input::Down;
                break;
            case SDLK_d:
                keyInput.type = Input::Right;
                break;
        }

        if (keyInput.type != Input::None) {
            EntityID player = world.playerID;
            Input::State& entityState = world.inputs[player].inputStates[keyInput.type];

            // If the state changed, save it and mark the player as dirty.
            if (entityState != keyInput.state) {
                entityState = keyInput.state;
                world.playerIsDirty = true;
            }
        }
    }
}

void PlayerInputSystem::addCurrentInputsToHistory()
{
    world.playerInputHistory.push(world.inputs[world.playerID].inputStates);
}

} // namespace Client
} // namespace AM
