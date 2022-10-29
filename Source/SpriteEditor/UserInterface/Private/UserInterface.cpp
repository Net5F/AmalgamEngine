#include "UserInterface.h"
#include "AssetCache.h"
#include "Config.h"
#include "AUI/Core.h"
#include "Log.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
UserInterface::UserInterface(SDL_Renderer* inRenderer, AssetCache& inAssetCache,
                             SpriteDataModel& inSpriteDataModel)
: auiInitializer{inRenderer,
                 {Config::LOGICAL_SCREEN_WIDTH, Config::LOGICAL_SCREEN_HEIGHT}}
, titleScreen{*this, inAssetCache, inSpriteDataModel}
, mainScreen{inAssetCache, inSpriteDataModel}
, currentScreen{&titleScreen}
{
    AUI::Core::setActualScreenSize(
        {Config::ACTUAL_SCREEN_WIDTH, Config::ACTUAL_SCREEN_HEIGHT});
}

void UserInterface::changeScreenTo(ScreenType screenType)
{
    switch (screenType) {
        case ScreenType::TitleScreen: {
            currentScreen = &titleScreen;
            break;
        }
        case ScreenType::MainScreen: {
            currentScreen = &mainScreen;
            break;
        }
        default: {
            currentScreen = &titleScreen;
            break;
        }
    }
}

void UserInterface::tick(double timestepS)
{
    // Let AUI process the next tick.
    currentScreen->tick(timestepS);
}

void UserInterface::render()
{
    if (currentScreen != nullptr) {
        currentScreen->render();
    }
}

bool UserInterface::handleOSEvent(SDL_Event& event)
{
    return currentScreen->handleOSEvent(event);
}

} // End namespace SpriteEditor
} // End namespace AM
