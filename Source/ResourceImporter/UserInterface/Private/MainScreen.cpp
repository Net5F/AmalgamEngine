#include "MainScreen.h"
#include "AssetCache.h"
#include "DataModel.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "nfd.h"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{
MainScreen::MainScreen(DataModel& inDataModel)
: AUI::Screen("MainScreen")
, dataModel{inDataModel}
, libraryWindow{*this, dataModel}
, libraryAddMenu{}
, saveButtonWindow{*this, dataModel}
, spriteEditStage{dataModel}
, iconEditStage{dataModel}
, spriteSetEditStage{dataModel, libraryWindow}
, spritePropertiesWindow{dataModel}
, spriteSetPropertiesWindow{dataModel}
, iconPropertiesWindow{dataModel}
, confirmationDialog{{0, 0, 1920, 1080}, "ConfirmationDialog"}
, addSpriteSheetDialog{dataModel}
, addIconSheetDialog{dataModel}
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(libraryWindow);
    windows.push_back(saveButtonWindow);
    windows.push_back(spriteEditStage);
    windows.push_back(spriteSetEditStage);
    windows.push_back(iconEditStage);
    windows.push_back(spritePropertiesWindow);
    windows.push_back(spriteSetPropertiesWindow);
    windows.push_back(iconPropertiesWindow);
    windows.push_back(libraryAddMenu);
    windows.push_back(confirmationDialog);
    windows.push_back(addSpriteSheetDialog);
    windows.push_back(addIconSheetDialog);

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
        Paths::TEXTURE_DIR + "MainButton/Normal.png");
    confirmationDialog.confirmButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "MainButton/Hovered.png");
    confirmationDialog.confirmButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "MainButton/Pressed.png");
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
        Paths::TEXTURE_DIR + "MainButton/Normal.png");
    confirmationDialog.cancelButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "MainButton/Hovered.png");
    confirmationDialog.cancelButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "MainButton/Pressed.png");
    confirmationDialog.cancelButton.text.setFont(
        (Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    confirmationDialog.cancelButton.text.setColor({255, 255, 255, 255});
    confirmationDialog.cancelButton.text.setText("CANCEL");

    // Set up the dialog's cancel button callback.
    confirmationDialog.cancelButton.setOnPressed([this]() {
        // Close the dialog.
        confirmationDialog.setIsVisible(false);
    });

    /* Library add menu. */
    libraryAddMenu.addSpriteSheetButton.setOnPressed([this]() {
        addSpriteSheetDialog.setIsVisible(true);
        // When we drop focus, the menu will close itself.
        dropFocus();
    });
    libraryAddMenu.addFloorButton.setOnPressed([this]() {
        dataModel.spriteSetModel.addFloor();
        dropFocus();
    });
    libraryAddMenu.addFloorCoveringButton.setOnPressed([this]() {
        dataModel.spriteSetModel.addFloorCovering();
        dropFocus();
    });
    libraryAddMenu.addWallButton.setOnPressed([this]() {
        dataModel.spriteSetModel.addWall();
        dropFocus();
    });
    libraryAddMenu.addObjectButton.setOnPressed([this]() {
        dataModel.spriteSetModel.addObject();
        dropFocus();
    });
    libraryAddMenu.addIconButton.setOnPressed([this]() {
        addIconSheetDialog.setIsVisible(true);
        dropFocus();
    });

    // Make the modal dialogs invisible.
    confirmationDialog.setIsVisible(false);
    addSpriteSheetDialog.setIsVisible(false);
    addIconSheetDialog.setIsVisible(false);
    libraryAddMenu.setIsVisible(false);

    /* Edit Stages and Properties Windows. */
    // Make the edit stages and properties windows invisible
    spriteEditStage.setIsVisible(false);
    spritePropertiesWindow.setIsVisible(false);
    spriteSetEditStage.setIsVisible(false);
    spriteSetPropertiesWindow.setIsVisible(false);
    iconEditStage.setIsVisible(false);
    iconPropertiesWindow.setIsVisible(false);

    // When the user selects a new item in the library, make the proper windows
    // visible.
    dataModel.activeLibraryItemChanged
        .connect<&MainScreen::onActiveLibraryItemChanged>(*this);
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

void MainScreen::openLibraryAddMenu()
{
    // If the menu isn't already open.
    if (!libraryAddMenu.getIsVisible()) {
        // Open the menu and focus it, so it can close itself if necessary.
        libraryAddMenu.setIsVisible(true);
        setFocusAfterNextLayout(&libraryAddMenu);
    }
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

void MainScreen::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Make everything invisible.
    spriteEditStage.setIsVisible(false);
    spritePropertiesWindow.setIsVisible(false);
    spriteSetEditStage.setIsVisible(false);
    spriteSetPropertiesWindow.setIsVisible(false);
    iconEditStage.setIsVisible(false);
    iconPropertiesWindow.setIsVisible(false);

    // Make the appropriate windows visible, based on the new item's type.
    if (std::holds_alternative<EditorSprite>(newActiveItem)) {
        spriteEditStage.setIsVisible(true);
        spritePropertiesWindow.setIsVisible(true);
    }
    else if (std::holds_alternative<EditorIcon>(newActiveItem)) {
        iconEditStage.setIsVisible(true);
        iconPropertiesWindow.setIsVisible(true);
    }
    else {
        // The new active item is a sprite set.
        spriteSetEditStage.setIsVisible(true);
        spriteSetPropertiesWindow.setIsVisible(true);
    }
}

} // End namespace ResourceImporter
} // End namespace AM
