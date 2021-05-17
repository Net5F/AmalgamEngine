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
, buttonContainer(*this, "ButtonContainer", {200, 200, 200, 600})
{
    // Set up our components.
    background.addResolution({1280, 720}, "Textures/TitleBackground_720.png");
    background.addResolution({1920, 1080}, "Textures/TitleBackground_1080.png");

    text.setFont("Fonts/B612-Regular.ttf", 20);
    text.setColor({255, 255, 255, 255});
    text.setText("This is temporary text.");

    loadButton.normalImage.addResolution({1280, 720}, "Textures/Button/Normal.png");
    loadButton.hoveredImage.addResolution({1280, 720}, "Textures/Button/Hovered.png");
    loadButton.pressedImage.addResolution({1280, 720}, "Textures/Button/Pressed.png");
    loadButton.disabledImage.addResolution({1280, 720}, "Textures/Button/Disabled.png");
    loadButton.text.setFont("Fonts/B612-Regular.ttf", 25);
    loadButton.text.setColor({255, 255, 255, 255});
    loadButton.text.setText("Load");

//    buttonContainer.setCellWidth(40);
//    buttonContainer.setCellHeight(40);
//    buttonContainer.setNumColumns(2);
    for (unsigned int i = 0; i < 5; ++i) {
        std::unique_ptr<AUI::Button> button
            = std::make_unique<AUI::Button>(*this, "", SDL_Rect(0, 0, 40, 40));
        button->normalImage.addResolution({1280, 720}, "Textures/Button/Normal.png");
        button->hoveredImage.addResolution({1280, 720}, "Textures/Button/Hovered.png");
        button->pressedImage.addResolution({1280, 720}, "Textures/Button/Pressed.png");
        button->text.setFont("Fonts/B612-Regular.ttf", 25);
        button->text.setColor({255, 255, 255, 255});
        button->text.setText(std::to_string(i));

        button->setOnPressed([i]() {
            AUI_LOG_INFO("Pressed %u", i);
        });

        buttonContainer.add(std::move(button));
    }

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

    buttonContainer.render();
}

} // End namespace SpriteEditor
} // End namespace AM
