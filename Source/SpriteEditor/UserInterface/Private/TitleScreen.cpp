#include "TitleScreen.h"
#include "UserInterface.h"
#include "AssetCache.h"
#include "AUI/Core.h"

namespace AM
{
namespace SpriteEditor
{
TitleScreen::TitleScreen(UserInterface& inUserInterface, AssetCache& assetCache,
                         SpriteDataModel& inSpriteDataModel)
: AUI::Screen("TitleScreen")
, titleWindow{inUserInterface, assetCache, *this, inSpriteDataModel}
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(titleWindow);
}

void TitleScreen::render()
{
    // Fill the background with the background color.
    SDL_Renderer* renderer{AUI::Core::getRenderer()};
    SDL_SetRenderDrawColor(renderer, 37, 37, 52, 255);
    SDL_RenderClear(renderer);

    // Update our window's layouts and render them.
    Screen::render();
}

} // End namespace SpriteEditor
} // End namespace AM
