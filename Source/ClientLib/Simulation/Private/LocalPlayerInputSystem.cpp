#include "LocalPlayerInputSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Input.h"
#include "InputHistory.h"
#include "InputChangeRequest.h"
#include "Camera.h"
#include "Config.h"
#include "AUI/Core.h"
#include "Log.h"

namespace AM
{
namespace Client
{
LocalPlayerInputSystem::LocalPlayerInputSystem(
    const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, currentZoomLevelIndex{Config::DEFAULT_ZOOM_LEVEL_INDEX}
{
}

void LocalPlayerInputSystem::processMomentaryInput(SDL_Event& event)
{
    switch (event.type) {
        case SDL_EVENT_MOUSE_WHEEL: {
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            const bool ctrlHeld{
                static_cast<bool>(event.key.mod & SDL_KMOD_CTRL)};

            // Handle zoom events.
            if (ctrlHeld && event.key.key == SDLK_EQUALS) {
                handleZoomIn();
            }
            else if (ctrlHeld && event.key.key == SDLK_MINUS) {
                handleZoomOut();
            }
            break;
        }
        default: {
            // No default behavior.
            break;
        }
    }
}

void LocalPlayerInputSystem::processHeldInputs()
{
    // If a text input UI widget is capturing keyboard input, do nothing.
    if (AUI::Core::getIsTextInputFocused()) {
        return;
    }

    const bool* keyStates{SDL_GetKeyboardState(nullptr)};
    Input::StateArr newInputStates{};

    // Get the latest state of all the keys that we care about.
    if (keyStates[SDL_SCANCODE_W]) {
        newInputStates[Input::YDown] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_A]) {
        newInputStates[Input::XDown] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_S]) {
        newInputStates[Input::YUp] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_D]) {
        newInputStates[Input::XUp] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_SPACE]) {
        newInputStates[Input::Jump] = Input::Pressed;
    }
    if (keyStates[SDL_SCANCODE_LCTRL]) {
        newInputStates[Input::Crouch] = Input::Pressed;
    }

    // Update our saved input state.
    Input& localPlayerInput{world.registry.get<Input>(world.localPlayerEntity)};
    bool inputHasChanged{false};
    for (std::size_t inputType{0}; inputType < Input::Type::Count;
         ++inputType) {
        // If the saved state doesn't match the latest.
        if (newInputStates[inputType]
            != localPlayerInput.inputStates[inputType]) {
            // Save the new state.
            localPlayerInput.inputStates[inputType] = newInputStates[inputType];
            inputHasChanged = true;
        }
    }

    // If our input state has changed, ask the server to apply the new state.
    if (inputHasChanged && !Config::RUN_OFFLINE) {
        network.worldEndpoint.send<InputChangeRequest>(
            {simulation.getCurrentTick(), localPlayerInput});
    }
}

void LocalPlayerInputSystem::addCurrentInputsToHistory()
{
    CircularBuffer<Input::StateArr,
                   InputHistory::LENGTH>& localPlayerInputHistory{
        world.registry.get<InputHistory>(world.localPlayerEntity).inputHistory};
    Input::StateArr& localPlayerInputs{
        world.registry.get<Input>(world.localPlayerEntity).inputStates};

    // Push the local player's current inputs into the local player's input
    // history.
    localPlayerInputHistory.push(localPlayerInputs);
}

void LocalPlayerInputSystem::handleZoomIn()
{
    // If zooming is disabled, do nothing.
    if (!Config::ENABLE_ZOOM) {
        return;
    }
    // If the local player doesn't have a camera, do nothing.
    else if (!(world.registry.all_of<Camera>(world.localPlayerEntity))) {
        return;
    }

    // Update the camera's current zoom level.
    if (currentZoomLevelIndex < (Config::ZOOM_LEVELS.size()) - 1) {
        currentZoomLevelIndex++;

        Camera& camera{world.registry.get<Camera>(world.localPlayerEntity)};
        camera.zoomFactor = Config::ZOOM_LEVELS.at(currentZoomLevelIndex);
    }
}

void LocalPlayerInputSystem::handleZoomOut()
{
    // If zooming is disabled, do nothing.
    if (!Config::ENABLE_ZOOM) {
        return;
    }
    // If the local player doesn't have a camera, do nothing.
    else if (!(world.registry.all_of<Camera>(world.localPlayerEntity))) {
        return;
    }

    // Update the camera's current zoom level.
    if (currentZoomLevelIndex > 0) {
        currentZoomLevelIndex--;

        Camera& camera{world.registry.get<Camera>(world.localPlayerEntity)};
        camera.zoomFactor = Config::ZOOM_LEVELS.at(currentZoomLevelIndex);
    }
}

} // namespace Client
} // namespace AM
