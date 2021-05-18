#include "TitleScreen.h"

namespace AM
{
namespace SpriteEditor
{

TitleScreen::TitleScreen()
: Screen("TitleScreen")
, background(*this, "Background", {0, 0, 1280, 720})
, newButton(*this, "NewButton", {483, 288, 314, 64}, "New")
, loadButton(*this, "LoadButton", {483, 393, 314, 64}, "Load")
{
    // Set up our components.
    background.addResolution({1280, 720}, "Textures/TitleBackground_720.png");
    background.addResolution({1920, 1080}, "Textures/TitleBackground_1080.png");

    // Register our event handlers.
    newButton.setOnPressed(std::bind(&TitleScreen::onNewButtonPressed, this));
    loadButton.setOnPressed(std::bind(&TitleScreen::onLoadButtonPressed, this));
}

void TitleScreen::render()
{
    background.render();

    newButton.render();

    loadButton.render();
}

void TitleScreen::onNewButtonPressed()
{
    AUI_LOG_INFO("Pressed New");
}

void TitleScreen::onLoadButtonPressed()
{
    AUI_LOG_INFO("Pressed Load");
}

} // End namespace SpriteEditor
} // End namespace AM
