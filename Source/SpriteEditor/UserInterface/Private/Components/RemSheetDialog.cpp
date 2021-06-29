#include "RemSheetDialog.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"

namespace AM
{
namespace SpriteEditor
{

RemSheetDialog::RemSheetDialog(MainScreen& inScreen, AUI::VerticalGridContainer& inSpriteSheetContainer
                               , AUI::Button& inRemSheetButton, SpriteDataModel& inSpriteDataModel)
: AUI::Component(screen, "", {0, 0, 1920, 1080})
, backgroundImage(screen, "", {0, 0, logicalExtent.w, logicalExtent.h})
, bodyText(screen, "", {763, 400, 400, 60})
, removeButton(screen, "", {1045, 520, 123, 56}, "REMOVE")
, cancelButton(screen, "", {903, 520, 123, 56}, "CANCEL")
, mainScreen{inScreen}
, spriteSheetContainer{inSpriteSheetContainer}
, remSheetButton{inRemSheetButton}
, spriteDataModel{inSpriteDataModel}
{
    /* Background image. */
    backgroundImage.addResolution({1920, 1080}, "Textures/Dialogs/RemSheetBackground.png");

    /* Body text. */
    bodyText.setFont("Fonts/B612-Regular.ttf", 21);
    bodyText.setColor({255, 255, 255, 255});
    bodyText.setText("Remove the selected sprite sheet and all associated sprites?");

    /* Buttons. */
    // Add a callback to remove a selected component on confirmation.
    removeButton.setOnPressed([&](){
        // Try to find a selected sprite sheet in the container.
        int selectedIndex{-1};
        for (unsigned int i = 0; i < spriteSheetContainer.size(); ++i) {
            MainThumbnail& thumbnail = static_cast<MainThumbnail&>(spriteSheetContainer[i]);
            if (thumbnail.getIsSelected()) {
                selectedIndex = i;
                break;
            }
        }

        // If we found a selected sprite sheet.
        if (selectedIndex != -1) {
            // Remove the sheet from the model.
            spriteDataModel.remSpriteSheet(selectedIndex);

            // Refresh the UI.
            mainScreen.loadSpriteData();

            // Disable the button since nothing is selected.
            remSheetButton.disable();
        }

        // Remove the dialog.
        setIsVisible(false);
    });

    // Add a callback to remove the dialog on cancel.
    cancelButton.setOnPressed([&](){
        // Remove the dialog.
        setIsVisible(false);
    });
}

void RemSheetDialog::render(const SDL_Point& parentOffset)
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
    bodyText.render(childOffset);
    removeButton.render(childOffset);
    cancelButton.render(childOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
