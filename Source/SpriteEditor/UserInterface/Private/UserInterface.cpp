#include "UserInterface.h"
#include "AUI/Image.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(ResourceManager& inResourceManager)
: currentPage()
, resourceManager(inResourceManager)
{
    // TODO: Load textures from a file, through a class that Application owns.
    // Load our textures.
    resourceManager.loadTexture("Resources/Textures", "TitleBackground_1080.png");
    resourceManager.loadTexture("Resources/Textures", "TitleBackground_720.png");

    AUI::Image backgroundImage(0, 0, 1080, 720
        , resourceManager.getTexture("TitleBackground_720.png").getSharedPtr()->Get());
    currentPage.addComponent(backgroundImage);
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
