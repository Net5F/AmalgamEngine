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
MainScreen::MainScreen(WorldSinks& inWorldSinks, EventDispatcher& inUiEventDispatcher, AssetCache& inAssetCache
                       , SpriteData& inSpriteData)
: AUI::Screen("MainScreen")
, buildOverlay{*this, inWorldSinks, inUiEventDispatcher}
, buildPanel{inAssetCache, *this, inSpriteData, buildOverlay}
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(buildOverlay);
    windows.push_back(buildPanel);
}

void MainScreen::setCamera(const Camera& inCamera)
{
    buildOverlay.setCamera(inCamera);
}

void MainScreen::render()
{
    // Update our child widget's layouts and render them.
    Screen::render();
}

} // End namespace Client
} // End namespace AM
