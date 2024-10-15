#include "SpriteSheetPropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "EditorSpriteSheet.h"
#include "StringTools.h"
#include "Paths.h"
#include "SharedConfig.h"
#include "nfd.hpp"

namespace AM
{
namespace ResourceImporter
{
SpriteSheetPropertiesWindow::SpriteSheetPropertiesWindow(MainScreen& inScreen,
                                                         DataModel& inDataModel)
: AUI::Window({1617, 0, 303, 579}, "SpriteSheetPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, addImagesButton{{24, 134, 255, 34}, "Add Images", "AddImagesButton"}
, mainScreen{inScreen}
, dataModel{inDataModel}
, activeSpriteSheetID{NULL_SPRITE_SHEET_ID}
, backgroundImage{{0, 0, 303, 579}, "PropertiesBackground"}
, headerImage{{0, 0, 303, 40}, "PropertiesHeader"}
, windowLabel{{12, 0, 282, 40}, "PropertiesWindowLabel"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(addImagesButton);

    /* Window setup */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});
    headerImage.setNineSliceImage((Paths::TEXTURE_DIR + "HeaderBackground.png"),
                                  {1, 1, 1, 1});

    auto styleLabel
        = [&](AUI::Text& label, const std::string& text, int fontSize) {
        label.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), fontSize);
        label.setColor({255, 255, 255, 255});
        label.setText(text);
    };
    styleLabel(windowLabel, "Sprite Sheet Properties", 21);
    windowLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    /* Display name entry. */
    styleLabel(nameLabel, "Name", 21);
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    auto styleTextInput = [&](AUI::TextInput& textInput) {
        textInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
        textInput.setPadding({0, 8, 0, 8});
    };
    styleTextInput(nameInput);
    nameInput.setOnTextCommitted([this]() {
        dataModel.spriteModel.setSpriteSheetDisplayName(activeSpriteSheetID,
                                                        nameInput.getText());
    });

    /* Image adding. */
    addImagesButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 16);
    addImagesButton.setOnPressed([&]() { onAddImagesButtonPressed(); });

    // When the active sprite sheet is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpriteSheetPropertiesWindow::onActiveLibraryItemChanged>(
            *this);
}

void SpriteSheetPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite sheet and return early if not.
    const EditorSpriteSheet* newActiveSpriteSheet{
        get_if<EditorSpriteSheet>(&newActiveItem)};
    if (!newActiveSpriteSheet) {
        activeSpriteSheetID = NULL_SPRITE_SHEET_ID;
        return;
    }

    activeSpriteSheetID = newActiveSpriteSheet->numericID;

    // Update all of our property fields to match the new active sheet's data.
    nameInput.setText(newActiveSpriteSheet->displayName);
}

void SpriteSheetPropertiesWindow::onAddImagesButtonPressed()
{
    // Open the file select dialog and save the selected paths.
    // Note: This will block the main thread, but that's fine. Our 
    //       PeriodicUpdaters in Application are set up to skip late steps.
    NFD::UniquePathSet selectedPaths{};
    nfdfilteritem_t filterItem[1] = {{"Supported Image Types", "png, jpg"}};
    nfdresult_t result = NFD::OpenDialogMultiple(selectedPaths, filterItem, 1);

    if (result == NFD_OKAY) {
        nfdpathsetsize_t numPaths;
        NFD::PathSet::Count(selectedPaths, numPaths);

        // Check that all of the paths start with the working IndividualSprites
        // directory.
        for (nfdpathsetsize_t i{0}; i < numPaths; ++i) {
            NFD::UniquePathSetPath path{};
            NFD::PathSet::GetPath(selectedPaths, i, path);

            if (!(StringTools::pathStartsWith(
                    path.get(), dataModel.getWorkingIndividualSpritesDir()))) {
                std::string errorString{
                    "Failed to add images: Sprite images must be placed in the "
                    "IndividualSprites directory."};
                mainScreen.openErrorDialog(errorString);
                return;
            }
        }

        // Add the sprite images to the model.
        std::size_t spritesDirSize{
            dataModel.getWorkingIndividualSpritesDir().size()};
        for (nfdpathsetsize_t i{0}; i < numPaths; ++i) {
            NFD::UniquePathSetPath path{};
            NFD::PathSet::GetPath(selectedPaths, i, path);

            // Trim the full path down to a path relative to the working 
            // IndividualSprites directory.
            std::string relPath{path.get()};
            relPath = relPath.substr(spritesDirSize);

            if (!(dataModel.spriteModel.addSprite(relPath,
                                                  activeSpriteSheetID))) {
                std::string errorString{"Failed to add sprite: "};
                errorString += dataModel.spriteModel.getErrorString();
                mainScreen.openErrorDialog(errorString);
                return;
            }
        }
    }
    else if (result != NFD_CANCEL) {
        // The dialog operation didn't succeed and the user didn't simply press
        // cancel. Print the error.
        LOG_INFO("Error: %s", NFD::GetError());
    }
}

} // End namespace ResourceImporter
} // End namespace AM
