#include "MainScreen.h"
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
, spriteSheetEditView{dataModel}
, boundingBoxEditView{dataModel, libraryWindow}
, spriteEditView{dataModel}
, animationElementsWindow{*this, dataModel}
, animationEditView{dataModel, libraryWindow, animationElementsWindow}
, iconEditView{dataModel}
, graphicSetEditView{dataModel, libraryWindow}
, entityGraphicSetEditView{dataModel, libraryWindow}
, spriteSheetPropertiesWindow{*this, dataModel}
, boundingBoxPropertiesWindow{dataModel, libraryWindow}
, spritePropertiesWindow{*this, dataModel, libraryWindow}
, animationPropertiesWindow{*this, dataModel, libraryWindow}
, graphicSetPropertiesWindow{dataModel}
, entityGraphicSetPropertiesWindow{dataModel}
, iconPropertiesWindow{dataModel}
, libraryAddMenu{}
, hamburgerButtonWindow{*this}
, hamburgerMenu{}
, confirmationDialog{{0, 0, 1920, 1080}, "ConfirmationDialog"}
, addSpriteDialog{*this, dataModel}
, addIconSheetDialog{dataModel}
, saveBoundingBoxDialog{dataModel}
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(libraryWindow);
    windows.push_back(spriteSheetEditView);
    windows.push_back(spriteSheetEditView);
    windows.push_back(boundingBoxEditView);
    windows.push_back(spriteEditView);
    windows.push_back(animationElementsWindow);
    windows.push_back(animationEditView);
    windows.push_back(graphicSetEditView);
    windows.push_back(entityGraphicSetEditView);
    windows.push_back(iconEditView);
    windows.push_back(spriteSheetPropertiesWindow);
    windows.push_back(boundingBoxPropertiesWindow);
    windows.push_back(spritePropertiesWindow);
    windows.push_back(animationPropertiesWindow);
    windows.push_back(graphicSetPropertiesWindow);
    windows.push_back(entityGraphicSetPropertiesWindow);
    windows.push_back(iconPropertiesWindow);
    windows.push_back(libraryAddMenu);
    windows.push_back(hamburgerButtonWindow);
    windows.push_back(hamburgerMenu);
    windows.push_back(confirmationDialog);
    windows.push_back(addSpriteDialog);
    windows.push_back(addIconSheetDialog);
    windows.push_back(saveBoundingBoxDialog);

    /* Confirmation dialog. */
    // Background shadow image.
    confirmationDialog.shadowImage.setLogicalExtent({0, 0, 1920, 1080});
    confirmationDialog.shadowImage.setSimpleImage(Paths::TEXTURE_DIR
                                                  + "Dialogs/Shadow.png");

    // Background image.
    confirmationDialog.backgroundImage.setLogicalExtent({710, 370, 500, 300});
    confirmationDialog.backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    // Body text.
    confirmationDialog.bodyText.setLogicalExtent({734, 390, 426, 132});
    confirmationDialog.bodyText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"),
                                        20);
    confirmationDialog.bodyText.setColor({255, 255, 255, 255});

    // Buttons.
    auto styleDialogButton = [&](AUI::Button& button, const SDL_Rect& logicalExtent) {
        button.setLogicalExtent(logicalExtent);
        SDL_Rect imageExtent{0, 0, logicalExtent.w, logicalExtent.h};
        button.normalImage.setLogicalExtent(imageExtent);
        button.hoveredImage.setLogicalExtent(imageExtent);
        button.pressedImage.setLogicalExtent(imageExtent);
        button.text.setLogicalExtent({-1, -1, logicalExtent.w, logicalExtent.h});
        button.normalImage.setNineSliceImage(
            Paths::TEXTURE_DIR + "MainButton/Normal.png", {4, 4, 4, 4});
        button.hoveredImage.setNineSliceImage(
            Paths::TEXTURE_DIR + "MainButton/Hovered.png", {4, 4, 4, 4});
        button.pressedImage.setNineSliceImage(
            Paths::TEXTURE_DIR + "MainButton/Pressed.png", {4, 4, 4, 4});
        button.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
        button.text.setColor({255, 255, 255, 255});
    };
    styleDialogButton(confirmationDialog.confirmButton, {1074, 604, 120, 50});
    styleDialogButton(confirmationDialog.cancelButton, {940, 604, 120, 50});
    confirmationDialog.cancelButton.text.setText("Cancel");

    // Set up the dialog's cancel button callback.
    confirmationDialog.cancelButton.setOnPressed([this]() {
        // Close the dialog.
        confirmationDialog.setIsVisible(false);
    });

    /* Library add menu. */
    libraryAddMenu.addSpriteSheetButton.setOnPressed([this]() {
        dataModel.spriteModel.addSpriteSheet();
        dropFocus();
    });
    libraryAddMenu.addTerrainButton.setOnPressed([this]() {
        dataModel.graphicSetModel.addTerrain();
        dropFocus();
    });
    libraryAddMenu.addFloorButton.setOnPressed([this]() {
        dataModel.graphicSetModel.addFloor();
        dropFocus();
    });
    libraryAddMenu.addWallButton.setOnPressed([this]() {
        dataModel.graphicSetModel.addWall();
        dropFocus();
    });
    libraryAddMenu.addObjectButton.setOnPressed([this]() {
        dataModel.graphicSetModel.addObject();
        dropFocus();
    });
    libraryAddMenu.addEntityButton.setOnPressed([this]() {
        dataModel.entityGraphicSetModel.addEntity();
        dropFocus();
    });
    libraryAddMenu.addIconSheetButton.setOnPressed([this]() {
        addIconSheetDialog.setIsVisible(true);
        dropFocus();
    });

    /* Hamburger menu. */
    hamburgerMenu.saveButton.setOnPressed([this]() {
        openConfirmationDialog("Save over existing ResourceData.json?", "Save",
                               [&]() { dataModel.save(); });
        dropFocus();
    });
    hamburgerMenu.exportButton.setOnPressed([this]() {
        openConfirmationDialog(
            "This will export sprite sheet image files into the SpriteSheets "
            "directory, overwriting any files currently present.\n\nProceed?",
            "Export",
            [&]() { dataModel.spriteModel.exportSpriteSheetImages(); });
        dropFocus();
    });

    // Make the modal dialogs invisible.
    libraryAddMenu.setIsVisible(false);
    hamburgerMenu.setIsVisible(false);
    confirmationDialog.setIsVisible(false);
    addSpriteDialog.setIsVisible(false);
    addIconSheetDialog.setIsVisible(false);
    saveBoundingBoxDialog.setIsVisible(false);

    // Make the edit stages and properties windows invisible
    spriteSheetEditView.setIsVisible(false);
    spriteSheetPropertiesWindow.setIsVisible(false);
    boundingBoxEditView.setIsVisible(false);
    boundingBoxPropertiesWindow.setIsVisible(false);
    spriteEditView.setIsVisible(false);
    spritePropertiesWindow.setIsVisible(false);
    animationElementsWindow.setIsVisible(false);
    animationEditView.setIsVisible(false);
    animationPropertiesWindow.setIsVisible(false);
    graphicSetEditView.setIsVisible(false);
    graphicSetPropertiesWindow.setIsVisible(false);
    entityGraphicSetEditView.setIsVisible(false);
    entityGraphicSetPropertiesWindow.setIsVisible(false);
    iconEditView.setIsVisible(false);
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
    // Set the dialog's text and make sure the cancel button is visible.
    confirmationDialog.bodyText.setText(bodyText);
    confirmationDialog.confirmButton.text.setText(confirmButtonText);
    confirmationDialog.cancelButton.setIsVisible(true);

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

void MainScreen::openErrorDialog(const std::string& bodyText)
{
    // Note: We just repurpose the confirmationDialog by hiding the cancel 
    //       button and using the confirm button to cancel.

    // Hide the "Cancel" button and repurpose the "Confirm" button for closing.
    confirmationDialog.cancelButton.setIsVisible(false);
    confirmationDialog.confirmButton.text.setText("Okay");
    confirmationDialog.confirmButton.setOnPressed([&]() {
        // Close the dialog.
        confirmationDialog.setIsVisible(false);
    });

    // Set the dialog's text.
    confirmationDialog.bodyText.setText(bodyText);

    // Open the dialog.
    confirmationDialog.setIsVisible(true);
}

void MainScreen::openSaveBoundingBoxDialog(
    const BoundingBox& modelBoundsToSave,
    std::function<void(BoundingBoxID)> saveCallback)
{
    saveBoundingBoxDialog.setSaveData(modelBoundsToSave,
                                      std::move(saveCallback));
    saveBoundingBoxDialog.setIsVisible(true);
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

void MainScreen::openHamburgerMenu()
{
    // If the menu isn't already open.
    if (!hamburgerMenu.getIsVisible()) {
        // Open the menu and focus it, so it can close itself if necessary.
        hamburgerMenu.setIsVisible(true);
        setFocusAfterNextLayout(&hamburgerMenu);
    }
}

void MainScreen::openAddSpriteDialog(
    const std::vector<std::string>& spriteImageRelPaths)
{
    // Pass the vector through and open the dialog.
    addSpriteDialog.setSpriteImageRelPaths(spriteImageRelPaths);
    addSpriteDialog.setIsVisible(true);
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
    spriteSheetEditView.setIsVisible(false);
    spriteSheetPropertiesWindow.setIsVisible(false);
    boundingBoxEditView.setIsVisible(false);
    boundingBoxPropertiesWindow.setIsVisible(false);
    spriteEditView.setIsVisible(false);
    spritePropertiesWindow.setIsVisible(false);
    animationElementsWindow.setIsVisible(false);
    animationEditView.setIsVisible(false);
    animationPropertiesWindow.setIsVisible(false);
    graphicSetEditView.setIsVisible(false);
    graphicSetPropertiesWindow.setIsVisible(false);
    entityGraphicSetEditView.setIsVisible(false);
    entityGraphicSetPropertiesWindow.setIsVisible(false);
    iconEditView.setIsVisible(false);
    iconPropertiesWindow.setIsVisible(false);

    // Make the appropriate windows visible, based on the new item's type.
    if (holds_alternative<EditorSpriteSheet>(newActiveItem)) {
        spriteSheetEditView.setIsVisible(true);
        spriteSheetPropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorBoundingBox>(newActiveItem)) {
        boundingBoxEditView.setIsVisible(true);
        boundingBoxPropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorSprite>(newActiveItem)) {
        spriteEditView.setIsVisible(true);
        spritePropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorAnimation>(newActiveItem)) {
        animationElementsWindow.setIsVisible(true);
        animationEditView.setIsVisible(true);
        animationPropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorEntityGraphicSet>(newActiveItem)) {
        entityGraphicSetEditView.setIsVisible(true);
        entityGraphicSetPropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorIcon>(newActiveItem)) {
        iconEditView.setIsVisible(true);
        iconPropertiesWindow.setIsVisible(true);
    }
    else {
        // The new active item is a non-entity graphic set.
        graphicSetEditView.setIsVisible(true);
        graphicSetPropertiesWindow.setIsVisible(true);
    }
}

} // End namespace ResourceImporter
} // End namespace AM
