#include "UserInterface.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(ResourceManager& inResourceManager)
: resourceManager(inResourceManager)
{
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
