#include "PlayerInputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace Client
{
PlayerInputSystem::PlayerInputSystem(Game& inGame, World& inWorld)
: game(inGame)
, world(inWorld)
{
}

void PlayerInputSystem::processMomentaryInput(SDL_Event& event)
{
    // We currently don't have any momentary inputs that we care about.
    ignore(event);
}

void PlayerInputSystem::processHeldInputs()
{
    const Uint8* keyStates = SDL_GetKeyboardState(nullptr);
    InputStateArr newInputStates{};

    // Get the latest state of all the keys that we care about.
    if (keyStates[SDL_SCANCODE_W]) {
        newInputStates[Input::Up] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_A]) {
        newInputStates[Input::Left] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_S]) {
        newInputStates[Input::Down] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_D]) {
        newInputStates[Input::Right] = Input::Pressed;
    }

    // Update our saved input state.
    InputStateArr& playerInputs = world.inputs[world.playerData.ID].inputStates;
    for (unsigned int inputType = 0; inputType < Input::Type::NumTypes; ++inputType) {
        // If the saved state doesn't match the latest.
        if (newInputStates[inputType] != playerInputs[inputType]) {
            // Save the new state.
            playerInputs[inputType] = newInputStates[inputType];

            // Mark the player as dirty.
            world.playerData.isDirty = true;
        }
    }
}

void PlayerInputSystem::addCurrentInputsToHistory()
{
    world.playerData.inputHistory.push(world.inputs[world.playerData.ID].inputStates);
}

} // namespace Client
} // namespace AM
