#include "TitleScreen.h"

namespace AM
{
namespace SpriteEditor
{

TitleScreen::TitleScreen()
: Screen("TitleScreen")
, background(*this, "Background", {0, 0, 1280, 720})
, text(*this, "Text", {300, 300, 40, 10})
, loadButton(*this, "LoadButton", {483, 393, 314, 64})
{
    // Set up our components.
    background.setImage("Textures/TitleBackground_720.png");

    text.setFont("Fonts/B612-Regular.ttf", 20);
    text.setColor({255, 255, 255, 255});
    text.setText("This is temporary text.");

    loadButton.normalImage.setImage("Textures/Button/Normal.png");
    loadButton.hoveredImage.setImage("Textures/Button/Hovered.png");
    loadButton.pressedImage.setImage("Textures/Button/Pressed.png");
    loadButton.disabledImage.setImage("Textures/Button/Disabled.png");
    loadButton.text.setFont("Fonts/B612-Regular.ttf", 25);
    loadButton.text.setColor({255, 255, 255, 255});
    loadButton.text.setText("Load");

    // Register our event handlers.
    loadButton.setOnPressed([]() {
        AUI_LOG_INFO("Pressed");
    });
}

void TitleScreen::render()
{
    background.render();

    text.render();

    loadButton.render();
}

} // End namespace SpriteEditor
} // End namespace AM
