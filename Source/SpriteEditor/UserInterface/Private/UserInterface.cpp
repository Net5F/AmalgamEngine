#include "UserInterface.h"
#include "AUI/Core.h"
#include "AUI/Image.h"
#include "Log.h"
#include "Ignore.h"
#include <SDL_filesystem.h>

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(SDL_Renderer* renderer)
: currentScreen()
{
    // Initialize AmalgamUI.
    AUI::Core::Initialize(SDL_GetBasePath(), renderer);

    AUI::Image& background = currentScreen.addComponent<AUI::Image>(
        "Background", {0, 0, 1280, 720});
    background.setImage("Resources/Textures/TitleBackground_720.png");
}

bool UserInterface::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEMOTION:
            handleMouseMotion(event.motion);

            // Temporarily consuming mouse events until the sim has some use
            // for them.
            return true;
        case SDL_MOUSEBUTTONDOWN:
            handleMouseButtonDown(event.button);
            return true;
    }

    return false;
}

void UserInterface::handleMouseMotion(SDL_MouseMotionEvent& event)
{
    ignore(event);
}

void UserInterface::handleMouseButtonDown(SDL_MouseButtonEvent& event)
{
    switch (event.button) {
        case SDL_BUTTON_LEFT:
            break;
    }
}

} // End namespace SpriteEditor
} // End namespace AM
