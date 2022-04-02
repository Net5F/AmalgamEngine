#include "TitleWindow.h"
#include "UserInterface.h"
#include "AssetCache.h"
#include "TitleScreen.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Log.h"
#include "nfd.hpp"
#include <cstring>

namespace AM
{
namespace SpriteEditor
{
TitleWindow::TitleWindow(UserInterface& inUserInterface,
                         AssetCache& inAssetCache,
                         SpriteDataModel& inSpriteDataModel)
: AUI::Window({0, 0, 1920, 1080}, "TitleWindow")
, userInterface{inUserInterface}
, assetCache{inAssetCache}
, spriteDataModel{inSpriteDataModel}
, titleText({0, 193, 1920, 75}, "TitleText")
, newButton(assetCache, {724, 432, 472, 96}, "New", "NewButton")
, loadButton(assetCache, {724, 589, 472, 96}, "Load", "LoadButton")
, errorText({0, 721, 1920, 48}, "ErrorText")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(titleText);
    children.push_back(newButton);
    children.push_back(loadButton);
    children.push_back(errorText);

    /* Title text. */
    titleText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 54);
    titleText.setColor({255, 255, 255, 255});
    titleText.setText("Amalgam Engine Sprite Editor");
    titleText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    /* Error text. */
    errorText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 36);
    errorText.setColor({255, 255, 255, 255});
    errorText.setText("Uninitialized.");
    errorText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    errorText.setIsVisible(false);

    // Register our event handlers.
    newButton.setOnPressed(std::bind(&TitleWindow::onNewButtonPressed, this));
    loadButton.setOnPressed(std::bind(&TitleWindow::onLoadButtonPressed, this));
}

void TitleWindow::onNewButtonPressed()
{
    // Open the file select dialog and save the selected path.
    nfdchar_t* selectedPath{nullptr};
    nfdresult_t result = NFD::PickFolder(selectedPath);

    if (result == NFD_OKAY) {
        // If we successfully created a new file, change to the main screen.
        std::string resultString{spriteDataModel.create(selectedPath)};
        if (resultString == "") {
            userInterface.openMainScreen();
        }
        else {
            // Failed to create file or dir, display the error text.
            resultString = "Error: " + resultString;
            errorText.setText(resultString);
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

void TitleWindow::onLoadButtonPressed()
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
            std::string resultString{spriteDataModel.load(selectedPath)};
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
