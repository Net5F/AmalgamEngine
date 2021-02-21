#include "PlayerInputSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Input.h"
#include "PlayerState.h"
#include "Camera.h"
#include "IsDirty.h"
#include "Log.h"

namespace AM
{
namespace Client
{
PlayerInputSystem::PlayerInputSystem(Simulation& inSim, World& inWorld)
: sim(inSim)
, world(inWorld)
{
}

void PlayerInputSystem::processMomentaryInput(SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEWHEEL:
            processMouseWheel(event.wheel);
            break;
        default:
            // No default behavior.
            break;
    }
}

void PlayerInputSystem::processMouseState(SDL_MouseMotionEvent& event)
{
    // Save the mouse's new position.
    world.mouseScreenPoint.x = event.x;
    world.mouseScreenPoint.y = event.y;

    // Account for the camera screen position and zoom factor.
    Camera& playerCamera = world.registry.get<Camera>(world.playerEntity);
    float screenX = (event.x + playerCamera.extent.x) / playerCamera.zoomFactor;
    float screenY = (event.y + playerCamera.extent.y) / playerCamera.zoomFactor;

    // Remove the half-tile X offset that we add to align tile sprites with
    // non-tile sprites.
    screenX -= (TILE_SCREEN_WIDTH / 2.f);

    // Calc the scaling factor going from screen tiles to world tiles.
    static const float TILE_WIDTH_SCALE = static_cast<float>(TILE_WORLD_WIDTH) / TILE_SCREEN_WIDTH;
    static const float TILE_HEIGHT_SCALE = static_cast<float>(TILE_WORLD_HEIGHT) / TILE_SCREEN_HEIGHT;

    // Calc the world position.
    float worldX = ((2 * screenY) + screenX) * (TILE_WIDTH_SCALE);
    float worldY = ((2 * screenY) - screenX) * (TILE_HEIGHT_SCALE) / 2;

    LOG_INFO("Mouse at screen: (%d, %d), world: (%.4f, %.4f)"
             , event.x, event.y, worldX, worldY);
}

void PlayerInputSystem::processHeldInputs()
{
    const Uint8* keyStates = SDL_GetKeyboardState(nullptr);
    Input::StateArr newInputStates{};

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
    entt::registry& registry = world.registry;
    Input& playerInput = registry.get<Input>(world.playerEntity);
    for (unsigned int inputType = 0; inputType < Input::Type::NumTypes;
         ++inputType) {
        // If the saved state doesn't match the latest.
        if (newInputStates[inputType] != playerInput.inputStates[inputType]) {
            // Save the new state.
            playerInput.inputStates[inputType] = newInputStates[inputType];

            // Flag the player as dirty if it isn't already.
            if (!(world.registry.has<IsDirty>(world.playerEntity))) {
                registry.emplace<IsDirty>(world.playerEntity);
            }
        }
    }
}

void PlayerInputSystem::addCurrentInputsToHistory()
{
    CircularBuffer<Input::StateArr, PlayerState::INPUT_HISTORY_LENGTH>&
        playerInputHistory
        = world.registry.get<PlayerState>(world.playerEntity).inputHistory;
    Input::StateArr& playerInputs
        = world.registry.get<Input>(world.playerEntity).inputStates;

    // Push the player's current inputs into the player's input history.
    playerInputHistory.push(playerInputs);
}

void PlayerInputSystem::processMouseWheel(SDL_MouseWheelEvent& wheelEvent)
{
    // Only process zoom if the player has a camera.
    if (world.registry.has<Camera>(world.playerEntity)) {
        /* Zoom the player's camera based on the mouse wheel movement. */
        Camera& camera = world.registry.get<Camera>(world.playerEntity);

        if (wheelEvent.y > 0) {
            // Zoom in a set amount per tick regardless of how much they
            // scrolled.
            camera.zoomFactor += camera.zoomSensitivity;
        }
        else {
            // Zoom out.
            if (camera.zoomFactor > camera.zoomSensitivity) {
                camera.zoomFactor -= camera.zoomSensitivity;
            }
        }
    }
}

} // namespace Client
} // namespace AM
