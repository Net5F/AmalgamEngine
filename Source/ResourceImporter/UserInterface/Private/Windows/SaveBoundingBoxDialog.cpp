#include "SaveBoundingBoxDialog.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "DataModel.h"
#include "Paths.h"

namespace AM
{
namespace ResourceImporter
{
SaveBoundingBoxDialog::SaveBoundingBoxDialog(DataModel& inDataModel)
: AUI::Window({0, 0, 1920, 1080}, "SaveBoundingBoxDialog")
, shadowImage({0, 0, logicalExtent.w, logicalExtent.h})
, backgroundImage({719, 208, 523, 506})
, headerText({747, 228, 400, 60})
, nameLabel({747, 300, 151, 38})
, nameInput({919, 300, 180, 38})
, descriptionText({747, 355, 485, 100})
, saveButton({1099, 640, 123, 56}, "Save")
, cancelButton({958, 640, 123, 56}, "Cancel")
, dataModel{inDataModel}
, modelBoundsToSave{}
, saveCallback{}
, errorText({748, 556, 466, 60})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(shadowImage);
    children.push_back(backgroundImage);
    children.push_back(headerText);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(descriptionText);
    children.push_back(saveButton);
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
    headerText.setText("Save bounding box");

    /* Name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Display Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});

    /* Description text. */
    descriptionText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    descriptionText.setColor({255, 255, 255, 255});
    descriptionText.setText("Warning: If a bounding box with the given name "
                            "exists, it will be overwritten.");

    /* Confirmation buttons. */
    // Add a callback to save the bounding box.
    saveButton.setOnPressed([this]() {
        // If the user-inputted data is valid.
        const std::string& name{nameInput.getText()};
        if (name != "") {
            // Update the model.
            BoundingBoxID boundingBoxID{
                dataModel.boundingBoxModel.addOrUpdateBoundingBox(
                    name, modelBoundsToSave)};

            // Call the callback.
            if (saveCallback) {
                saveCallback(boundingBoxID);
            }

            // Clear this dialog's text to prepare for the next usage.
            clear();

            // Remove the dialog.
            errorText.setIsVisible(false);
            setIsVisible(false);
        }
        else {
            // Data wasn't valid, display an error string.
            errorText.setText("Name field must not be empty.");
            errorText.setIsVisible(true);
        }
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
}

void SaveBoundingBoxDialog::setSaveData(
    const BoundingBox& inModelBoundsToSave,
    const std::function<void(BoundingBoxID)> inSaveCallback)
{
    modelBoundsToSave = inModelBoundsToSave;
    saveCallback = std::move(inSaveCallback);
}

void SaveBoundingBoxDialog::clear()
{
    nameInput.setText("");
    errorText.setText("");
}

} // End namespace ResourceImporter
} // End namespace AM
