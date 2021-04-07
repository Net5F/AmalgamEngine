#include "UserInterface.h"
#include "AUI/Core.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "Log.h"
#include "Ignore.h"
#include <SDL_filesystem.h>

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(SDL_Renderer* renderer)
: initializer((std::string{SDL_GetBasePath()} + "Resources/"), renderer)
, currentScreen("TitleScreen")
{
    AUI::Image& background = currentScreen.add<AUI::Image>(
        "Background", {0, 0, 1280, 720});
    background.setImage("Textures/TitleBackground_720.png");

    AUI::Text& text = currentScreen.add<AUI::Text>(
        "Text", {40, 40, 200, 200});
    text.setFont("Fonts/B612-Regular.ttf", 40);
    text.setColor({255, 255, 255, 255});
    text.setRenderMode(AUI::Text::RenderMode::Blended);
    text.setText("This is temporary text.");
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
