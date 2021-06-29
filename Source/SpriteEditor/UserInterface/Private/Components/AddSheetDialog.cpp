#include "AddSheetDialog.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"

namespace AM
{
namespace SpriteEditor
{

AddSheetDialog::AddSheetDialog(MainScreen& inScreen, AUI::VerticalGridContainer& inSpriteSheetContainer, SpriteDataModel& inSpriteDataModel)
: AUI::Component(inScreen, "", {0, 0, 1920, 1080})
, backgroundImage(inScreen, "", {0, 0, logicalExtent.w, logicalExtent.h})
, headerText(inScreen, "", {747, 228, 280, 60})
, pathLabel(inScreen, "", {747, 300, 151, 38})
, pathInput(inScreen, "", {919, 300, 180, 38})
, widthLabel(inScreen, "", {747, 350, 151, 38})
, widthInput(inScreen, "", {919, 350, 180, 38})
, heightLabel(inScreen, "", {747, 400, 151, 38})
, heightInput(inScreen, "", {919, 400, 180, 38})
, nameLabel(inScreen, "", {747, 450, 151, 38})
, nameInput(inScreen, "", {919, 450, 180, 38})
, addButton(inScreen, "", {1099, 593, 123, 56}, "ADD")
, cancelButton(inScreen, "", {958, 593, 123, 56}, "CANCEL")
, mainScreen{inScreen}
, spriteSheetContainer{inSpriteSheetContainer}
, spriteDataModel{inSpriteDataModel}
{
    /* Background image. */
    backgroundImage.addResolution({1920, 1080}, "Textures/Dialogs/AddSheetBackground.png");

    /* Header text. */
    headerText.setFont("Fonts/B612-Regular.ttf", 32);
    headerText.setColor({255, 255, 255, 255});
    headerText.setText("Add sprite sheet");

    /* Path entry. */
    pathLabel.setFont("Fonts/B612-Regular.ttf", 21);
    pathLabel.setColor({255, 255, 255, 255});
    pathLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    pathLabel.setText("Relative Path");

    pathInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    pathInput.setMargins({8, 0, 8, 0});

    /* Width entry. */
    widthLabel.setFont("Fonts/B612-Regular.ttf", 21);
    widthLabel.setColor({255, 255, 255, 255});
    widthLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    widthLabel.setText("Sprite Width");

    widthInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    widthInput.setMargins({8, 0, 8, 0});

    /* Height entry. */
    heightLabel.setFont("Fonts/B612-Regular.ttf", 21);
    heightLabel.setColor({255, 255, 255, 255});
    heightLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    heightLabel.setText("Sprite Height");

    heightInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    heightInput.setMargins({8, 0, 8, 0});

    /* Name entry. */
    nameLabel.setFont("Fonts/B612-Regular.ttf", 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Base Name");

    nameInput.setTextFont("Fonts/B612-Regular.ttf", 18);
    nameInput.setMargins({8, 0, 8, 0});

    /* Confirmation buttons. */
    // Add a callback to validate the input and add the new sprite sheet.
    addButton.setOnPressed([this](){
        // Pass the user-inputted data to the model.
        std::string result = spriteDataModel.addSpriteSheet(pathInput.getText(), widthInput.getText()
                    , heightInput.getText(), nameInput.getText());

        // If the data was valid.
        if (result == "") {
            // Refresh the UI.
            mainScreen.loadSpriteData();

            // Clear the text inputs.
            clearTextInputs();

            // Remove the dialog.
            setIsVisible(false);
        }
        else {
            // Data wasn't valid, display an error string.
            // TODO: Add an error text
            LOG_ERROR("%s", result.c_str());
        }
    });

    // Add a callback to remove the dialog on cancel.
    cancelButton.setOnPressed([&](){
        // Clear the text inputs.
        clearTextInputs();

        // Remove the dialog.
        setIsVisible(false);
    });
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

    // Render the children.
    backgroundImage.render(childOffset);

    headerText.render(childOffset);

    pathLabel.render(childOffset);
    pathInput.render(childOffset);

    widthLabel.render(childOffset);
    widthInput.render(childOffset);

    heightLabel.render(childOffset);
    heightInput.render(childOffset);

    nameLabel.render(childOffset);
    nameInput.render(childOffset);

    addButton.render(childOffset);
    cancelButton.render(childOffset);
}

void AddSheetDialog::clearTextInputs()
{
    pathInput.setText("");
    widthInput.setText("");
    heightInput.setText("");
    nameInput.setText("");
}

} // End namespace SpriteEditor
} // End namespace AM
