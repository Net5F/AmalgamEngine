#include "UserInterface.h"
#include "Config.h"
#include "AUI/Core.h"
#include "Log.h"
#include "Ignore.h"
#include <SDL_filesystem.h>

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(SDL_Renderer* renderer)
: initializer((std::string{SDL_GetBasePath()} + "Resources/"), renderer
              , {Config::LOGICAL_SCREEN_WIDTH, Config::LOGICAL_SCREEN_HEIGHT})
, currentScreen(&titleScreen)
{
    AUI::Core::setActualScreenSize({Config::ACTUAL_SCREEN_WIDTH, Config::ACTUAL_SCREEN_HEIGHT});
}

bool UserInterface::handleEvent(SDL_Event& event)
{
    return currentScreen->handleEvent(event);
}

} // End namespace SpriteEditor
} // End namespace AM
