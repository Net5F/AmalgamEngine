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
, boundingBoxEditView{dataModel, libraryWindow}
, boundingBoxPropertiesWindow{dataModel, libraryWindow}
, spriteEditView{dataModel}
, animationEditView{dataModel, libraryWindow}
, iconEditView{dataModel}
, graphicSetEditView{dataModel, libraryWindow}
, entityGraphicSetEditView{dataModel, libraryWindow}
, spritePropertiesWindow{dataModel, libraryWindow}
, animationPropertiesWindow{*this, dataModel, libraryWindow}
, graphicSetPropertiesWindow{dataModel}
, entityGraphicSetPropertiesWindow{dataModel}
, iconPropertiesWindow{dataModel}
, confirmationDialog{{0, 0, 1920, 1080}, "ConfirmationDialog"}
, addSpriteSheetDialog{dataModel}
, addIconSheetDialog{dataModel}
, saveBoundingBoxDialog{dataModel}
{
    // Add our windows so they're included in rendering, etc.
    windows.push_back(libraryWindow);
    windows.push_back(saveButtonWindow);
    windows.push_back(boundingBoxEditView);
    windows.push_back(boundingBoxPropertiesWindow);
    windows.push_back(spriteEditView);
    windows.push_back(animationEditView);
    windows.push_back(graphicSetEditView);
    windows.push_back(entityGraphicSetEditView);
    windows.push_back(iconEditView);
    windows.push_back(spritePropertiesWindow);
    windows.push_back(animationPropertiesWindow);
    windows.push_back(graphicSetPropertiesWindow);
    windows.push_back(entityGraphicSetPropertiesWindow);
    windows.push_back(iconPropertiesWindow);
    windows.push_back(libraryAddMenu);
    windows.push_back(confirmationDialog);
    windows.push_back(addSpriteSheetDialog);
    windows.push_back(addIconSheetDialog);
    windows.push_back(saveBoundingBoxDialog);

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
    styleDialogButton(confirmationDialog.confirmButton, {1045, 520, 123, 56});
    styleDialogButton(confirmationDialog.cancelButton, {903, 520, 123, 56});
    confirmationDialog.cancelButton.text.setText("Cancel");

    // Set up the dialog's cancel button callback.
    confirmationDialog.cancelButton.setOnPressed([this]() {
        // Close the dialog.
        confirmationDialog.setIsVisible(false);
    });

    /* Library add menu. */
    libraryAddMenu.addBoundingBoxButton.setOnPressed([this]() {
        dataModel.boundingBoxModel.addBoundingBox();
        // When we drop focus, the menu will close itself.
        dropFocus();
    });
    libraryAddMenu.addSpriteSheetButton.setOnPressed([this]() {
        addSpriteSheetDialog.setIsVisible(true);
        dropFocus();
    });
    libraryAddMenu.addAnimationButton.setOnPressed([this]() {
        dataModel.animationModel.addAnimation();
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
    libraryAddMenu.addIconButton.setOnPressed([this]() {
        addIconSheetDialog.setIsVisible(true);
        dropFocus();
    });

    // Make the modal dialogs invisible.
    libraryAddMenu.setIsVisible(false);
    confirmationDialog.setIsVisible(false);
    addSpriteSheetDialog.setIsVisible(false);
    addIconSheetDialog.setIsVisible(false);
    saveBoundingBoxDialog.setIsVisible(false);

    /* Edit Stages and Properties Windows. */
    // Make the edit stages and properties windows invisible
    boundingBoxEditView.setIsVisible(false);
    boundingBoxPropertiesWindow.setIsVisible(false);
    spriteEditView.setIsVisible(false);
    spritePropertiesWindow.setIsVisible(false);
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
    boundingBoxEditView.setIsVisible(false);
    boundingBoxPropertiesWindow.setIsVisible(false);
    spriteEditView.setIsVisible(false);
    spritePropertiesWindow.setIsVisible(false);
    animationEditView.setIsVisible(false);
    animationPropertiesWindow.setIsVisible(false);
    graphicSetEditView.setIsVisible(false);
    graphicSetPropertiesWindow.setIsVisible(false);
    entityGraphicSetEditView.setIsVisible(false);
    entityGraphicSetPropertiesWindow.setIsVisible(false);
    iconEditView.setIsVisible(false);
    iconPropertiesWindow.setIsVisible(false);

    // Make the appropriate windows visible, based on the new item's type.
    if (holds_alternative<EditorBoundingBox>(newActiveItem)) {
        boundingBoxEditView.setIsVisible(true);
        boundingBoxPropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorSprite>(newActiveItem)) {
        spriteEditView.setIsVisible(true);
        spritePropertiesWindow.setIsVisible(true);
    }
    else if (holds_alternative<EditorAnimation>(newActiveItem)) {
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
