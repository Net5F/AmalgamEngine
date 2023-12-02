#include "UserInterface.h"
#include "AssetCache.h"
#include "Config.h"
#include "AUI/Core.h"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{
UserInterface::UserInterface(SDL_Renderer* inRenderer, AssetCache& inAssetCache,
                             DataModel& inDataModel)
: auiInitializer{inRenderer,
                 {Config::LOGICAL_SCREEN_WIDTH, Config::LOGICAL_SCREEN_HEIGHT}}
, titleScreen{*this, inDataModel}
, mainScreen{inDataModel}
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

} // End namespace ResourceImporter
} // End namespace AM
