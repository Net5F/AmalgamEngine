#include "TitleScreen.h"

namespace AM
{
namespace SpriteEditor
{

TitleScreen::TitleScreen()
: Screen("TitleScreen")
, background(*this, "Background", {0, 0, 1280, 720})
, text(*this, "Text", {400, 400, 40, 10})
, loadButton(*this, "LoadButton", {483, 393, 314, 64})
{
    background.setImage("Textures/TitleBackground_720.png");

    text.setFont("Fonts/B612-Regular.ttf", 20);
    text.setColor({255, 255, 255, 255});
    text.setRenderMode(AUI::Text::RenderMode::Blended);
    text.setText("This is temporary text.");

    loadButton.normalImage.setImage("Textures/Button/Normal.png");
    loadButton.hoveredImage.setImage("Textures/Button/Hovered.png");
    loadButton.pressedImage.setImage("Textures/Button/Pressed.png");
    loadButton.disabledImage.setImage("Textures/Button/Disabled.png");
    loadButton.text.setFont("Fonts/B612-Regular.ttf", 25);
    loadButton.text.setColor({255, 255, 255, 255});
    loadButton.text.setRenderMode(AUI::Text::RenderMode::Blended);
    loadButton.text.setText("Load");
}

void TitleScreen::render()
{
    background.render();

    text.render();

    loadButton.render();
}

} // End namespace SpriteEditor
} // End namespace AM
