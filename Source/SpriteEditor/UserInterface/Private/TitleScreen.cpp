#include "TitleScreen.h"

namespace AM
{
namespace SpriteEditor
{

TitleScreen::TitleScreen()
: Screen("TitleScreen")
, background(*this, "Background", {0, 0, 1280, 720})
, text(*this, "Text", {40, 40, 200, 200})
{
    background.setImage("Textures/TitleBackground_720.png");

    text.setFont("Fonts/B612-Regular.ttf", 40);
    text.setColor({255, 255, 255, 255});
    text.setRenderMode(AUI::Text::RenderMode::Blended);
    text.setText("This is temporary text.");
}

void TitleScreen::render()
{
    background.render();

    text.render();
}

} // End namespace SpriteEditor
} // End namespace AM
