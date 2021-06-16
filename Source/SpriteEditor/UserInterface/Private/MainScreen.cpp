#include "TitleScreen.h"
#include "UserInterface.h"
#include "AUI/Core.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{

MainScreen::MainScreen(UserInterface& inUserInterface)
: Screen("MainScreen")
, userInterface(inUserInterface)
, spritesheetPanel(*this)
, textInput(*this, "", {400, 400, 300, 100})
{
    textInput.setTextFont("Fonts/B612-Regular.ttf", 25);
    textInput.setText("Hello thereqwjeoiqwjeoiqwejioqwe");
    textInput.setMargins({10, 10, 10, 10});
    textInput.setCursorWidth(2);
    textInput.normalImage.addResolution({1280, 720}, "Textures/TextBox/Normal.png");
    textInput.hoveredImage.addResolution({1280, 720}, "Textures/TextBox/Hovered.png");
    textInput.selectedImage.addResolution({1280, 720}, "Textures/TextBox/Selected.png");
    textInput.disabledImage.addResolution({1280, 720}, "Textures/TextBox/Disabled.png");
    textInput.setOnTextChanged([](){
        AUI_LOG_INFO("Text changed.");
    });
    textInput.setOnTextCommitted([](){
        AUI_LOG_INFO("Text committed.");
    });
}

void MainScreen::loadSpriteFile(const std::string& filePath)
{
    // Store the file path so we can eventually save back to it.
    currentSpriteFilePath = filePath;
}

void MainScreen::render()
{
    // Fill the background with the correct color.
    SDL_Renderer* renderer = AUI::Core::GetRenderer();
    SDL_SetRenderDrawColor(renderer, 17, 17, 19, 255);
    SDL_RenderClear(renderer);

    // Render our children.
    spritesheetPanel.render();

    textInput.render();
}

} // End namespace SpriteEditor
} // End namespace AM
