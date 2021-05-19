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
, tempText(*this, "", {200, 200, 200, 200})
{
    // Set up our components.
    tempText.setFont("Fonts/B612-Regular.ttf", 25);
    tempText.setColor({255, 255, 255, 255});
}

void MainScreen::loadSpriteFile(const std::string& filePath)
{
    tempText.setText(filePath);
}

void MainScreen::render()
{
    tempText.render();
}

} // End namespace SpriteEditor
} // End namespace AM
