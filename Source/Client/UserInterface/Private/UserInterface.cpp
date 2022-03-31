#include "UserInterface.h"
#include "Config.h"
#include "World.h"
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
UserInterface::UserInterface(EventDispatcher& inUiEventDispatcher, const World& inWorld,
                             SDL_Renderer* inSDLRenderer, AssetCache& inAssetCache,
                             SpriteData& inSpriteData)
: world{inWorld}
, auiInitializer{inSDLRenderer,
                 {Config::LOGICAL_SCREEN_WIDTH, Config::LOGICAL_SCREEN_HEIGHT}}
, mainScreen{inUiEventDispatcher, inAssetCache, inSpriteData}
, currentScreen{&mainScreen}
{
    AUI::Core::setActualScreenSize(
        {Config::ACTUAL_SCREEN_WIDTH, Config::ACTUAL_SCREEN_HEIGHT});
}

bool UserInterface::handleOSEvent(SDL_Event& event)
{
    return currentScreen->handleOSEvent(event);
}

void UserInterface::tick(double timestepS) {
    currentScreen->tick(timestepS);
}

void UserInterface::render(const Camera& camera)
{
    mainScreen.setCamera(camera);
    // TODO: This needs to be changed to a signal.
    mainScreen.setTileMapExtent(world.tileMap.getTileExtent());

    currentScreen->render();
}

} // End namespace Client
} // End namespace AM
