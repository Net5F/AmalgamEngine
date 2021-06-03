#include "TitleScreen.h"
#include "UserInterface.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{

MainScreen::MainScreen(UserInterface& inUserInterface)
: Screen("MainScreen")
, userInterface(inUserInterface)
//, tempText(*this, "", {200, 200, 200, 200})
, textBox(*this, "", {400, 400, 300, 100})
{
    // Set up our components.
//    tempText.setFont("Fonts/B612-Regular.ttf", 25);
//    tempText.setColor({255, 255, 255, 255});

    textBox.setCursorColor({255, 255, 255, 255});
    textBox.setTextFont("Fonts/B612-Regular.ttf", 25);
    textBox.setTextColor({255, 0, 0, 255});
    textBox.setText("Hello thereqwjeoiqwjeoiqwejioqwe");
    textBox.setMargins({10, 10, 10, 10});
    textBox.normalImage.addResolution({1280, 720}, "Textures/TextBox/Normal.png");
    textBox.hoveredImage.addResolution({1280, 720}, "Textures/TextBox/Hovered.png");
    textBox.selectedImage.addResolution({1280, 720}, "Textures/TextBox/Selected.png");
    textBox.disabledImage.addResolution({1280, 720}, "Textures/TextBox/Disabled.png");
}

void MainScreen::loadSpriteFile(const std::string& filePath)
{
//    tempText.setText(filePath);

    // Store the file path so we can eventually save back to it.
    currentSpriteFilePath = filePath;
}

void MainScreen::render()
{
//    tempText.render();

    textBox.render();
}

} // End namespace SpriteEditor
} // End namespace AM
