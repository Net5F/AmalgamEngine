#include "TitleScreen.h"
#include "UserInterface.h"
#include "AUI/Core.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{

MainScreen::MainScreen(SpriteDataModel& inSpriteDataModel)
: Screen("MainScreen")
, spriteDataModel{inSpriteDataModel}
, spritesheetPanel(*this, spriteDataModel)
{
}

void MainScreen::loadSpriteData()
{
    // For every sprite sheet in the model.
//    for (const SpriteSheet& sheet : spriteDataModel.getSpriteSheets()) {
//        // Add a Thumbnail component that displays the sheet.
//        spritesheetPanel.addSpriteSheet(sheet.relPath);
//    }
}

void MainScreen::render()
{
    // Fill the background with the correct color.
    SDL_Renderer* renderer = AUI::Core::GetRenderer();
    SDL_SetRenderDrawColor(renderer, 17, 17, 19, 255);
    SDL_RenderClear(renderer);

    // Render our children.
    spritesheetPanel.render();
}

} // End namespace SpriteEditor
} // End namespace AM
