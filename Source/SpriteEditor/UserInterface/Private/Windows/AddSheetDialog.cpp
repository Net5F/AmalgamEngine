#include "AddSheetDialog.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
AddSheetDialog::AddSheetDialog(SpriteDataModel& inSpriteDataModel)
: AUI::Window({0, 0, 1920, 1080}, "AddSheetDialog")
, backgroundImage({0, 0, logicalExtent.w, logicalExtent.h})
, headerText({747, 228, 280, 60})
, pathLabel({747, 300, 151, 38})
, pathInput({919, 300, 180, 38})
, widthLabel({747, 350, 151, 38})
, widthInput({919, 350, 180, 38})
, heightLabel({747, 400, 151, 38})
, heightInput({919, 400, 180, 38})
, offsetLabel({747, 450, 151, 38})
, offsetInput({919, 450, 180, 38})
, nameLabel({747, 500, 151, 38})
, nameInput({919, 500, 180, 38})
, addButton({1099, 640, 123, 56}, "ADD")
, cancelButton({958, 640, 123, 56}, "CANCEL")
, spriteDataModel{inSpriteDataModel}
, errorText({748, 556, 466, 60})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerText);
    children.push_back(pathLabel);
    children.push_back(pathInput);
    children.push_back(widthLabel);
    children.push_back(widthInput);
    children.push_back(heightLabel);
    children.push_back(heightInput);
    children.push_back(offsetLabel);
    children.push_back(offsetInput);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(addButton);
    children.push_back(cancelButton);
    children.push_back(errorText);

    /* Background image. */
    backgroundImage.setSimpleImage(Paths::TEXTURE_DIR + "Dialogs/AddSheetBackground.png");

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
    pathInput.setPadding({0, 8, 0, 8});

    /* Width entry. */
    widthLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    widthLabel.setColor({255, 255, 255, 255});
    widthLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    widthLabel.setText("Sprite Width");

    widthInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    widthInput.setPadding({0, 8, 0, 8});

    /* Height entry. */
    heightLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    heightLabel.setColor({255, 255, 255, 255});
    heightLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    heightLabel.setText("Sprite Height");

    heightInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    heightInput.setPadding({0, 8, 0, 8});

    /* Y offset entry. */
    offsetLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    offsetLabel.setColor({255, 255, 255, 255});
    offsetLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    offsetLabel.setText("Y Offset");

    offsetInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    offsetInput.setPadding({0, 8, 0, 8});

    /* Name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Base Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});

    /* Confirmation buttons. */
    // Add a callback to validate the input and add the new sprite sheet.
    addButton.setOnPressed([this]() {
        // Pass the user-inputted data to the model.
        std::string result{spriteDataModel.addSpriteSheet(
            pathInput.getText(), widthInput.getText(), heightInput.getText(),
            offsetInput.getText(), nameInput.getText())};

        // If the data was valid.
        if (result == "") {
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
    cancelButton.setOnPressed([&]() {
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

void AddSheetDialog::clear()
{
    pathInput.setText("");
    widthInput.setText("");
    heightInput.setText("");
    offsetInput.setText("");
    nameInput.setText("");
    errorText.setText("");
}

} // End namespace SpriteEditor
} // End namespace AM
