#include "RemSheetDialog.h"
#include "MainThumbnail.h"

namespace AM {

RemSheetDialog::RemSheetDialog(AUI::Screen& screen, AUI::VerticalGridContainer& inSpritesheetContainer, AUI::Button& inRemSheetButton)
: AUI::Component(screen, "", {0, 0, 1920, 1080})
, backgroundImage(screen, "", {0, 0, logicalExtent.w, logicalExtent.h})
, bodyText(screen, "", {763, 400, 400, 60})
, removeButton(screen, "", {1045, 520, 123, 56}, "REMOVE")
, cancelButton(screen, "", {903, 520, 123, 56}, "CANCEL")
, spritesheetContainer(inSpritesheetContainer)
, remSheetButton(inRemSheetButton)
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
        AUI::Component* selectedSheet{nullptr};
        for (auto& componentPtr : spritesheetContainer) {
            MainThumbnail& thumbnail = static_cast<MainThumbnail&>(*componentPtr);
            if (thumbnail.getIsSelected()) {
                selectedSheet = &thumbnail;
                break;
            }
        }

        // If we found a selected sprite sheet, erase it.
        if (selectedSheet != nullptr) {
            spritesheetContainer.erase(selectedSheet);

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

} // namespace AUI
