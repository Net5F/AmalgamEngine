#include "MainScreen.h"
#include "AssetCache.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace SpriteEditor
{
MainScreen::MainScreen(SpriteDataModel& inSpriteDataModel)
: AUI::Screen("MainScreen")
, spriteDataModel{inSpriteDataModel}
, libraryWindow(*this, spriteDataModel)
, spriteEditStage(spriteDataModel)
, saveButtonWindow(*this, spriteDataModel)
, propertiesPanel(spriteDataModel)
, confirmationDialog({0, 0, 1920, 1080}, "ConfirmationDialog")
, addSheetDialog(spriteDataModel)
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(spriteEditStage);
    windows.push_back(libraryWindow);
    windows.push_back(saveButtonWindow);
    windows.push_back(propertiesPanel);
    windows.push_back(confirmationDialog);
    windows.push_back(addSheetDialog);

    /* Confirmation dialog. */
    // Background shadow image.
    confirmationDialog.shadowImage.setLogicalExtent({0, 0, 1920, 1080});
    confirmationDialog.shadowImage.setSimpleImage(Paths::TEXTURE_DIR
                                                  + "Dialogs/Shadow.png");

    // Background image.
    confirmationDialog.backgroundImage.setLogicalExtent({721, 358, 474, 248});
    confirmationDialog.backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    // Body text.
    confirmationDialog.bodyText.setLogicalExtent({763, 400, 400, 60});
    confirmationDialog.bodyText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"),
                                        21);
    confirmationDialog.bodyText.setColor({255, 255, 255, 255});

    // Buttons.
    confirmationDialog.confirmButton.setLogicalExtent({1045, 520, 123, 56});
    confirmationDialog.confirmButton.normalImage.setLogicalExtent(
        {0, 0, 123, 56});
    confirmationDialog.confirmButton.hoveredImage.setLogicalExtent(
        {0, 0, 123, 56});
    confirmationDialog.confirmButton.pressedImage.setLogicalExtent(
        {0, 0, 123, 56});
    confirmationDialog.confirmButton.text.setLogicalExtent({-1, -1, 123, 56});
    confirmationDialog.confirmButton.normalImage.setSimpleImage(
        Paths::TEXTURE_DIR + "ConfirmationButton/Normal.png");
    confirmationDialog.confirmButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "ConfirmationButton/Hovered.png");
    confirmationDialog.confirmButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "ConfirmationButton/Pressed.png");
    confirmationDialog.confirmButton.text.setFont(
        (Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    confirmationDialog.confirmButton.text.setColor({255, 255, 255, 255});

    confirmationDialog.cancelButton.setLogicalExtent({903, 520, 123, 56});
    confirmationDialog.cancelButton.normalImage.setLogicalExtent(
        {0, 0, 123, 56});
    confirmationDialog.cancelButton.hoveredImage.setLogicalExtent(
        {0, 0, 123, 56});
    confirmationDialog.cancelButton.pressedImage.setLogicalExtent(
        {0, 0, 123, 56});
    confirmationDialog.cancelButton.text.setLogicalExtent({-1, -1, 123, 56});
    confirmationDialog.cancelButton.normalImage.setSimpleImage(
        Paths::TEXTURE_DIR + "ConfirmationButton/Normal.png");
    confirmationDialog.cancelButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "ConfirmationButton/Hovered.png");
    confirmationDialog.cancelButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "ConfirmationButton/Pressed.png");
    confirmationDialog.cancelButton.text.setFont(
        (Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    confirmationDialog.cancelButton.text.setColor({255, 255, 255, 255});
    confirmationDialog.cancelButton.text.setText("CANCEL");

    // Set up the dialog's cancel button callback.
    confirmationDialog.cancelButton.setOnPressed([this]() {
        // Close the dialog.
        confirmationDialog.setIsVisible(false);
    });

    // Make the modal dialogs invisible.
    confirmationDialog.setIsVisible(false);
    addSheetDialog.setIsVisible(false);
}

void MainScreen::openConfirmationDialog(
    const std::string& bodyText, const std::string& confirmButtonText,
    std::function<void(void)> onConfirmation)
{
    // Set the dialog's text.
    confirmationDialog.bodyText.setText(bodyText);
    confirmationDialog.confirmButton.text.setText(confirmButtonText);

    // Set the dialog's confirmation callback.
    userOnConfirmation = std::move(onConfirmation);
    confirmationDialog.confirmButton.setOnPressed([&]() {
        // Call the user's callback.
        userOnConfirmation();

        // Close the dialog.
        confirmationDialog.setIsVisible(false);
    });

    // Open the dialog.
    confirmationDialog.setIsVisible(true);
}

void MainScreen::openAddSheetDialog()
{
    // Open the dialog.
    addSheetDialog.setIsVisible(true);
}

void MainScreen::render()
{
    // Fill the background with the background color.
    SDL_Renderer* renderer{AUI::Core::getRenderer()};
    SDL_SetRenderDrawColor(renderer, 35, 35, 38, 255);
    SDL_RenderClear(renderer);

    // Update our child widget's layouts and render them.
    Screen::render();
}

} // End namespace SpriteEditor
} // End namespace AM
