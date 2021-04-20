#include "UserInterface.h"
#include "Log.h"
#include "Ignore.h"
#include <SDL_filesystem.h>

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(SDL_Renderer* renderer)
: initializer((std::string{SDL_GetBasePath()} + "Resources/"), renderer)
, currentScreen(&titleScreen)
{
}

bool UserInterface::handleEvent(SDL_Event& event)
{
    return currentScreen->handleEvent(event);
}

} // End namespace SpriteEditor
} // End namespace AM
