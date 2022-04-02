#include "UserInterface.h"
#include "Config.h"
#include "WorldSinks.h"
#include "AssetCache.h"
#include "SpriteData.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "ClientTransforms.h"
#include "Log.h"
#include "AUI/Core.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
UserInterface::UserInterface(WorldSinks& inWorldSinks,
                             EventDispatcher& inUiEventDispatcher,
                             SDL_Renderer* inSDLRenderer,
                             AssetCache& inAssetCache, SpriteData& inSpriteData)
: auiInitializer{inSDLRenderer,
                 {Config::LOGICAL_SCREEN_WIDTH, Config::LOGICAL_SCREEN_HEIGHT}}
, mainScreen{inWorldSinks, inUiEventDispatcher, inAssetCache, inSpriteData}
, currentScreen{&mainScreen}
{
    AUI::Core::setActualScreenSize(
        {Config::ACTUAL_SCREEN_WIDTH, Config::ACTUAL_SCREEN_HEIGHT});
}

bool UserInterface::handleOSEvent(SDL_Event& event)
{
    return currentScreen->handleOSEvent(event);
}

void UserInterface::tick(double timestepS)
{
    currentScreen->tick(timestepS);
}

void UserInterface::render(const Camera& camera)
{
    mainScreen.setCamera(camera);

    currentScreen->render();
}

} // End namespace Client
} // End namespace AM
