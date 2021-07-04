#include "TitleScreen.h"
#include "UserInterface.h"
#include "AUI/Core.h"
#include "nfd.hpp"
#include "Log.h"
#include <cstring>

namespace AM
{
namespace SpriteEditor
{

TitleScreen::TitleScreen(UserInterface& inUserInterface, SpriteDataModel& inSpriteDataModel)
: Screen("TitleScreen")
, userInterface{inUserInterface}
, spriteDataModel{inSpriteDataModel}
, titleText(*this, "TitleText", {0, 193, 1920, 75})
, newButton(*this, "NewButton", {724, 432, 472, 96}, "New")
, loadButton(*this, "LoadButton", {724, 589, 472, 96}, "Load")
, errorText(*this, "ErrorText", {0, 721, 1920, 48})
{
    /* Title text. */
    titleText.setFont("Fonts/B612-Regular.ttf", 54);
    titleText.setColor({255, 255, 255, 255});
    titleText.setText("Amalgam Engine Sprite Editor");
    titleText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    /* Error text. */
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
    // Fill the background with the background color.
    SDL_Renderer* renderer = AUI::Core::GetRenderer();
    SDL_SetRenderDrawColor(renderer, 37, 37, 52, 255);
    SDL_RenderClear(renderer);

    titleText.render();

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
        // If we successfully created a new file, change to the main screen.
        if (spriteDataModel.create(selectedPath)) {
            userInterface.openMainScreen();
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
        // Validate the selected file's name.
        if (std::strstr(selectedPath, "SpriteData.json") != 0) {
            // Valid file name.
            // If it loads successfully, change to the main screen.
            std::string resultString = spriteDataModel.load(selectedPath);
            if (resultString == "") {
                userInterface.openMainScreen();
            }
            else {
                // Failed to parse, display the error text.
                resultString = "Error: " + resultString;
                errorText.setText(resultString);
                errorText.setIsVisible(true);
            }
        }
        else {
            // Invalid file name, display the error text.
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
