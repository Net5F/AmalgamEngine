#include "TitleScreen.h"
#include "UserInterface.h"
#include "nfd.hpp"
#include "Log.h"
#include <cstring>
#include <fstream>

namespace AM
{
namespace SpriteEditor
{

TitleScreen::TitleScreen(UserInterface& inUserInterface)
: Screen("TitleScreen")
, userInterface(inUserInterface)
, background(*this, "Background", {0, 0, 1920, 1080})
, newButton(*this, "NewButton", {724, 432, 472, 96}, "New")
, loadButton(*this, "LoadButton", {724, 589, 472, 96}, "Load")
, errorText(*this, "ErrorText", {0, 721, 1920, 48})
{
    // Set up our components.
    background.addResolution({1280, 720}, "Textures/TitleBackground_720.png");
    background.addResolution({1920, 1080}, "Textures/TitleBackground_1080.png");

    errorText.setFont("Fonts/B612-Regular.ttf", 36);
    errorText.setColor({255, 255, 255, 255});
    errorText.setText("Uninitialized.");
    errorText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    errorText.setIsVisible(false);

    // Register our event handlers.
    newButton.setOnPressed(std::bind(&TitleScreen::onNewButtonPressed, this));
    loadButton.setOnPressed(std::bind(&TitleScreen::onLoadButtonPressed, this));
}

void TitleScreen::render()
{
    background.render();

    newButton.render();

    loadButton.render();

    errorText.render();
}

void TitleScreen::onNewButtonPressed()
{
    // Open the file select dialog and save the selected path.
    nfdchar_t* selectedPath{nullptr};
    nfdresult_t result = NFD::PickFolder(selectedPath);

    if (result == NFD_OKAY) {
        // Check if the file exists at the selected path.
        std::string filePath{selectedPath};
        filePath += "/SpriteData.json";
        std::ifstream file{filePath};
        if (!file) {
            // File doesn't exist, create it and change to the main screen.
            std::ofstream file{filePath};
            userInterface.openMainScreen(filePath);
        }
        else {
            // File already exists. Display the error text.
            errorText.setText("Error: SpriteData.json already exists at the "
                              "selected path.");
            errorText.setIsVisible(true);
        }

        NFD::FreePath(selectedPath);
    }
    else if (result != NFD_CANCEL) {
        // The dialog operation didn't succeed and the user didn't simply press
        // cancel. Print the error.
        LOG_INFO("Error: %s", NFD_GetError());
    }
}

void TitleScreen::onLoadButtonPressed()
{
    // New attempt, make sure the error text is hidden.
    errorText.setIsVisible(false);

    // Open the file select dialog and save the selected path.
    nfdchar_t* selectedPath{nullptr};
    nfdfilteritem_t filterItem[1] = {{"SpriteData.json", "json"}};
    nfdresult_t result = NFD::OpenDialog(selectedPath, filterItem, 1);

    if (result == NFD_OKAY) {
        // Validate the selected file.
        // Note: All we do here is check the name. It'll be further validated
        //       by the main screen.
        if (std::strstr(selectedPath, "SpriteData.json") != 0) {
            // Correct file, pass the file path and change to the main screen.
            userInterface.openMainScreen(selectedPath);
        }
        else {
            // Incorrect file, display the error text.
            errorText.setText("Error: Must select a SpriteData.json file to"
                              " load.");
            errorText.setIsVisible(true);
        }

        NFD::FreePath(selectedPath);
    }
    else if (result != NFD_CANCEL) {
        // The dialog operation didn't succeed and the user didn't simply press
        // cancel. Print the error.
        LOG_INFO("Error: %s", NFD_GetError());
    }
}

} // End namespace SpriteEditor
} // End namespace AM
