#include "TitleWindow.h"
#include "UserInterface.h"
#include "TitleScreen.h"
#include "DataModel.h"
#include "Paths.h"
#include "Log.h"
#include "nfd.hpp"
#include <cstring>

namespace AM
{
namespace ResourceImporter
{
TitleWindow::TitleWindow(UserInterface& inUserInterface, DataModel& inDataModel)
: AUI::Window({0, 0, 1920, 1080}, "TitleWindow")
, userInterface{inUserInterface}
, dataModel{inDataModel}
, titleText({0, 193, 1920, 75}, "TitleText")
, directionText({0, 482, 1920, 300}, "DirectionText")
, openButton({724, 548, 472, 96}, "Open", "OpenButton")
, errorText({20, 721, 1880, 300}, "ErrorText")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(titleText);
    children.push_back(directionText);
    children.push_back(openButton);
    children.push_back(errorText);

    /* Title text. */
    titleText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 54);
    titleText.setColor({255, 255, 255, 255});
    titleText.setText("Amalgam Engine Resource Importer");
    titleText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    /* Other text. */
    directionText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 28);
    directionText.setColor({255, 255, 255, 255});
    directionText.setText("Please locate your project's Resources directory:");
    directionText.setHorizontalAlignment(
        AUI::Text::HorizontalAlignment::Center);

    errorText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 28);
    errorText.setColor({255, 255, 255, 255});
    errorText.setText("Uninitialized.");
    errorText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    errorText.setIsVisible(false);

    // Register our event handlers.
    openButton.setOnPressed(std::bind(&TitleWindow::onOpenButtonPressed, this));
}

void TitleWindow::onOpenButtonPressed()
{
    // New attempt, make sure the error text is hidden.
    errorText.setIsVisible(false);

    // Open the file select dialog and save the selected path.
    // Note: This will block the main thread, but that's fine. Our 
    //       PeriodicUpdaters in Application are set up to skip late steps.
    nfdchar_t* selectedPath{nullptr};
    nfdresult_t result = NFD::PickFolder(selectedPath);

    if (result == NFD_OKAY) {
        // Try to open the project.
        if (dataModel.open(selectedPath)) {
            userInterface.changeScreenTo(UserInterface::ScreenType::MainScreen);
        }
        else {
            // Failed to open project, display the error text.
            errorText.setText("Error: " + dataModel.getErrorString());
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

} // End namespace ResourceImporter
} // End namespace AM
