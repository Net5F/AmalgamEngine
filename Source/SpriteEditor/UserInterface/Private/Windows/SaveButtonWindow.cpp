#include "SaveButtonWindow.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "Paths.h"

namespace AM
{
namespace SpriteEditor
{
SaveButtonWindow::SaveButtonWindow(MainScreen& inScreen,
                                   SpriteDataModel& inSpriteDataModel)
: AUI::Window({1537, 0, 58, 58}, "SaveButtonWindow")
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, saveButton({0, 0, 58, 58})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(saveButton);

    /* Save button. */
    saveButton.normalImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "SaveButton/Normal.png");
    saveButton.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                           + "SaveButton/Hovered.png");
    saveButton.pressedImage.setSimpleImage(Paths::TEXTURE_DIR
                                           + "SaveButton/Pressed.png");
    saveButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    saveButton.text.setText("");

    // Add a callback to save the current sprite data when pressed.
    saveButton.setOnPressed([this]() {
        // Create our callback.
        std::function<void(void)> onConfirmation = [&]() {
            // Save the data.
            spriteDataModel.save();
        };

        // Open the confirmation dialog.
        mainScreen.openConfirmationDialog("Save over existing SpriteData.json?",
                                          "SAVE", std::move(onConfirmation));
    });
}

} // End namespace SpriteEditor
} // End namespace AM
