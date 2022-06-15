#include "MainScreen.h"
#include "AssetCache.h"
#include "SpriteData.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "Log.h"

namespace AM
{
namespace Client
{
MainScreen::MainScreen(WorldSinks& inWorldSinks,
                       EventDispatcher& inUiEventDispatcher,
                       AssetCache& inAssetCache, SpriteData& inSpriteData)
: AUI::Screen("MainScreen")
, buildOverlay{inWorldSinks, inUiEventDispatcher}
, buildPanel{inAssetCache, inSpriteData, buildOverlay}
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(buildOverlay);
    windows.push_back(buildPanel);

    // Deactivate build mode.
    buildOverlay.setIsVisible(false);
    buildPanel.setIsVisible(false);
}

void MainScreen::setCamera(const Camera& inCamera)
{
    buildOverlay.setCamera(inCamera);
}

bool MainScreen::onKeyDown(SDL_Keycode keyCode)
{
    // If the 'b' key is pressed, toggle build mode.
    if (keyCode == SDLK_b) {
        bool buildModeIsActive{buildOverlay.getIsVisible()};

        buildOverlay.setIsVisible(!buildModeIsActive);
        buildPanel.setIsVisible(!buildModeIsActive);

        return true;
    }

    return false;
}

void MainScreen::render()
{
    // Update our child widget's layouts and render them.
    Screen::render();
}

} // End namespace Client
} // End namespace AM
