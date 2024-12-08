#include "AddSpriteDialog.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "DataModel.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
AddSpriteDialog::AddSpriteDialog(MainScreen& inMainScreen,
                                 DataModel& inDataModel)
: AUI::Window({0, 0, 1920, 1080}, "AddSpriteDialog")
, shadowImage({0, 0, logicalExtent.w, logicalExtent.h})
, backgroundImage({719, 208, 523, 506})
, headerText({747, 228, 280, 60})
, stageOriginXLabel({747, 300, 210, 38})
, stageOriginXInput({967, 300, 180, 38})
, stageOriginYLabel({747, 350, 210, 38})
, stageOriginYInput({967, 350, 180, 38})
, premultiplyAlphaLabel({747, 400, 210, 38})
, premultiplyAlphaInput({967, 408, 22, 22})
, addButton({1098, 640, 120, 50}, "Add")
, cancelButton({958, 640, 120, 50}, "Cancel")
, mainScreen{inMainScreen}
, dataModel{inDataModel}
, activeSpriteSheetID{NULL_SPRITE_SHEET_ID}
, spriteImageRelPaths{}
, errorText({747, 408, 466, 60})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(shadowImage);
    children.push_back(backgroundImage);
    children.push_back(headerText);
    children.push_back(stageOriginXLabel);
    children.push_back(stageOriginXInput);
    children.push_back(stageOriginYLabel);
    children.push_back(stageOriginYInput);
    children.push_back(premultiplyAlphaLabel);
    children.push_back(premultiplyAlphaInput);
    children.push_back(addButton);
    children.push_back(cancelButton);
    children.push_back(errorText);

    /* Background shadow image. */
    shadowImage.setSimpleImage(Paths::TEXTURE_DIR + "Dialogs/Shadow.png");

    /* Background image. */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});

    /* Header text. */
    headerText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 32);
    headerText.setColor({255, 255, 255, 255});
    headerText.setText("Add sprite");

    /* Stage origin entry. */
    auto styleLabel
        = [&](AUI::Text& label, const std::string& text, int fontSize) {
        label.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), fontSize);
        label.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
        label.setColor({255, 255, 255, 255});
        label.setText(text);
    };
    styleLabel(stageOriginXLabel, "Stage Origin X", 21);

    auto styleTextInput = [&](AUI::TextInput& textInput) {
        textInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
        textInput.setPadding({0, 8, 0, 8});
    };
    styleTextInput(stageOriginXInput);

    styleLabel(stageOriginYLabel, "Stage Origin Y", 21);
    styleTextInput(stageOriginYInput);

    /* Premultiply alpha entry. */
    styleLabel(premultiplyAlphaLabel, "Premultiply alpha", 21);

    premultiplyAlphaInput.uncheckedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Checkbox/Unchecked.png");
    premultiplyAlphaInput.checkedImage.setSimpleImage(Paths::TEXTURE_DIR
                                                      + "Checkbox/Checked.png");
    premultiplyAlphaInput.setOnChecked([this]() {});
    premultiplyAlphaInput.setOnUnchecked([this]() {});

    /* Confirmation buttons. */
    // Add a callback to validate the input and add the new sprites.
    addButton.setOnPressed([this]() {
        // Pass the user-inputted data to the model.
        bool allSpritesAdded{true};
        for (const std::string& spriteImageRelPath : spriteImageRelPaths) {
            bool premultiplyAlpha{premultiplyAlphaInput.getCurrentState()
                                  == AUI::Checkbox::State::Checked};
            if (!(dataModel.spriteModel.addSprite(
                    spriteImageRelPath, activeSpriteSheetID,
                    stageOriginXInput.getText(), stageOriginYInput.getText(),
                    premultiplyAlpha))) {
                std::string errorString{"Failed to add sprite: "};
                errorString += dataModel.spriteModel.getErrorString();
                mainScreen.openErrorDialog(errorString);
                break;
            }
        }

        // Clear this dialog's text to prepare for the next usage.
        clear();

        // Remove the dialog.
        errorText.setIsVisible(false);
        setIsVisible(false);
    });

    // Add a callback to remove the dialog on cancel.
    cancelButton.setOnPressed([&]() {
        // Clear the text inputs and labels.
        clear();

        // Remove the dialog.
        setIsVisible(false);
    });

    /* Error text. */
    errorText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 20);
    errorText.setColor({255, 255, 255, 255});
    errorText.setText("Uninitialized.");
    errorText.setIsVisible(false);

    // When the active sprite sheet is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&AddSpriteDialog::onActiveLibraryItemChanged>(*this);
}

void AddSpriteDialog::setSpriteImageRelPaths(
    const std::vector<std::string>& inSpriteImageRelPaths)
{
    spriteImageRelPaths = inSpriteImageRelPaths;

    std::string headerString{"Add "};
    headerString += std::to_string(spriteImageRelPaths.size());
    headerString += " sprite(s)";
    headerText.setText(headerString);
}

void AddSpriteDialog::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // If the new active item is a sprite sheet, save its ID.
    const EditorSpriteSheet* newActiveSpriteSheet{
        get_if<EditorSpriteSheet>(&newActiveItem)};
    if (newActiveSpriteSheet) {
        activeSpriteSheetID = newActiveSpriteSheet->numericID;
    }
    else {
        activeSpriteSheetID = NULL_SPRITE_SHEET_ID;
    }
}

void AddSpriteDialog::clear()
{
    stageOriginXInput.setText("");
    stageOriginYInput.setText("");
    errorText.setText("");
    premultiplyAlphaInput.setCurrentState(AUI::Checkbox::State::Unchecked);
}

} // End namespace ResourceImporter
} // End namespace AM
