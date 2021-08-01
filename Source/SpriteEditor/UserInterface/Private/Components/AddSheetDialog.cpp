#include "AddSheetDialog.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "AssetCache.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{

AddSheetDialog::AddSheetDialog(AssetCache& assetCache, MainScreen& inScreen, AUI::VerticalGridContainer& inSpriteSheetContainer, SpriteDataModel& inSpriteDataModel)
: AUI::Component(inScreen, {0, 0, 1920, 1080})
, backgroundImage(inScreen, {0, 0, logicalExtent.w, logicalExtent.h})
, headerText(inScreen, {747, 228, 280, 60})
, pathLabel(inScreen, {747, 300, 151, 38})
, pathInput(assetCache, inScreen, {919, 300, 180, 38})
, widthLabel(inScreen, {747, 350, 151, 38})
, widthInput(assetCache, inScreen, {919, 350, 180, 38})
, heightLabel(inScreen, {747, 400, 151, 38})
, heightInput(assetCache, inScreen, {919, 400, 180, 38})
, offsetLabel(inScreen, {747, 450, 151, 38})
, offsetInput(assetCache, inScreen, {919, 450, 180, 38})
, nameLabel(inScreen, {747, 500, 151, 38})
, nameInput(assetCache, inScreen, {919, 500, 180, 38})
, addButton(assetCache, inScreen, {1099, 640, 123, 56}, "ADD")
, cancelButton(assetCache, inScreen, {958, 640, 123, 56}, "CANCEL")
, mainScreen{inScreen}
, spriteSheetContainer{inSpriteSheetContainer}
, spriteDataModel{inSpriteDataModel}
, errorText(inScreen, {748, 556, 466, 60})
{
    /* Background image. */
    backgroundImage.addResolution({1920, 1080}, assetCache.loadTexture(
        Paths::TEXTURE_DIR + "Dialogs/AddSheetBackground.png"));

    /* Header text. */
    headerText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 32);
    headerText.setColor({255, 255, 255, 255});
    headerText.setText("Add sprite sheet");

    /* Path entry. */
    pathLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    pathLabel.setColor({255, 255, 255, 255});
    pathLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    pathLabel.setText("Relative Path");

    pathInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    pathInput.setMargins({8, 0, 8, 0});

    /* Width entry. */
    widthLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    widthLabel.setColor({255, 255, 255, 255});
    widthLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    widthLabel.setText("Sprite Width");

    widthInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    widthInput.setMargins({8, 0, 8, 0});

    /* Height entry. */
    heightLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    heightLabel.setColor({255, 255, 255, 255});
    heightLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    heightLabel.setText("Sprite Height");

    heightInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    heightInput.setMargins({8, 0, 8, 0});

    /* Y offset entry. */
    offsetLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    offsetLabel.setColor({255, 255, 255, 255});
    offsetLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    offsetLabel.setText("Y Offset");

    offsetInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    offsetInput.setMargins({8, 0, 8, 0});

    /* Name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Base Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setMargins({8, 0, 8, 0});

    /* Confirmation buttons. */
    // Add a callback to validate the input and add the new sprite sheet.
    addButton.setOnPressed([this](){
        // Pass the user-inputted data to the model.
        std::string result = spriteDataModel.addSpriteSheet(pathInput.getText(), widthInput.getText()
                    , heightInput.getText(), offsetInput.getText(), nameInput.getText());

        // If the data was valid.
        if (result == "") {
            // Refresh the UI.
            mainScreen.loadSpriteData();

            // Clear this dialog's text to prepare for the next usage.
            clear();

            // Remove the dialog.
            errorText.setIsVisible(false);
            setIsVisible(false);
        }
        else {
            // Data wasn't valid, display an error string.
            errorText.setText(result);
            errorText.setIsVisible(true);
        }
    });

    // Add a callback to remove the dialog on cancel.
    cancelButton.setOnPressed([&](){
        // Clear the text inputs.
        clear();

        // Remove the dialog.
        setIsVisible(false);
    });

    /* Error text. */
    errorText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 20);
    errorText.setColor({255, 255, 255, 255});
    errorText.setText("Uninitialized.");
    errorText.setIsVisible(false);
}

void AddSheetDialog::render(const SDL_Point& parentOffset)
{
    // Keep our extent up to date.
    refreshScaling();

    // Save the extent that we're going to render at.
    lastRenderedExtent = scaledExtent;
    lastRenderedExtent.x += parentOffset.x;
    lastRenderedExtent.y += parentOffset.y;

    // If the component isn't visible, return without rendering.
    if (!isVisible) {
        return;
    }

    // Children should render at the parent's offset + this component's offset.
    SDL_Point childOffset{parentOffset};
    childOffset.x += scaledExtent.x;
    childOffset.y += scaledExtent.y;

    // Render our children.
    backgroundImage.render(childOffset);

    headerText.render(childOffset);

    pathLabel.render(childOffset);
    pathInput.render(childOffset);

    widthLabel.render(childOffset);
    widthInput.render(childOffset);

    heightLabel.render(childOffset);
    heightInput.render(childOffset);

    offsetLabel.render(childOffset);
    offsetInput.render(childOffset);

    nameLabel.render(childOffset);
    nameInput.render(childOffset);

    addButton.render(childOffset);
    cancelButton.render(childOffset);

    errorText.render(childOffset);
}

void AddSheetDialog::clear()
{
    pathInput.setText("");
    widthInput.setText("");
    heightInput.setText("");
    nameInput.setText("");
    errorText.setText("");
}

} // End namespace SpriteEditor
} // End namespace AM
