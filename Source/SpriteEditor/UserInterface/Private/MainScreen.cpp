#include "TitleScreen.h"
#include "UserInterface.h"
#include "MainThumbnail.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{

MainScreen::MainScreen(UserInterface& inUserInterface)
: Screen("MainScreen")
, userInterface(inUserInterface)
, spritesheetContainer(*this, "SpritesheetContainer", {100, 100, 350, 600})
, textInput(*this, "", {400, 400, 300, 100})
, removeButton(*this, "", {850, 300, 314, 64}, "Remove")
{
    // Set up our components.
    for (unsigned int i = 0; i < 3; ++i) {
        std::unique_ptr<AUI::Component> thumbnail{
            std::make_unique<MainThumbnail>(*this, ""
                , SDL_Rect{0, 0, 100, 100})};
        MainThumbnail& thumbRef{static_cast<MainThumbnail&>(*thumbnail)};

        thumbRef.thumbnailImage.addResolution({1280, 720}, "Textures/Temp/iso_test_sprites.png");
        thumbRef.setText("Component " + std::to_string(i));

        spritesheetContainer.add(std::move(thumbnail));
    }

    // Remove a component on button push
    removeButton.setOnPressed([&](){
        AUI::Container& spriteContainer{
            this->get<AUI::Container>("SpritesheetContainer")};
        spriteContainer.erase(0);
    });

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
    spritesheetContainer.render();

    textInput.render();

    removeButton.render();
}

} // End namespace SpriteEditor
} // End namespace AM
