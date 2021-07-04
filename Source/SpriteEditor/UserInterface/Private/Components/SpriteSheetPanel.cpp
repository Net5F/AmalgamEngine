#include "SpriteSheetPanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{

SpriteSheetPanel::SpriteSheetPanel(MainScreen& inScreen, SpriteDataModel& inSpriteDataModel)
: AUI::Component(inScreen, "SpriteSheetPanel", {0, 0, 399, 708})
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, backgroundImage(inScreen, "", {0, 0, 399, 708})
, spriteSheetContainer(inScreen, "SpriteSheetContainer", {18, 24, 306, 650})
, remSheetButton(inScreen, "", {342, 0, 45, 63})
, addSheetButton(inScreen, "", {342, 63, 45, 88})
, addSheetDialog(inScreen, spriteSheetContainer, spriteDataModel)
{
    /* Background image */
    backgroundImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/Background_1920.png"
                                  , {4, 4, 399, 708});
    backgroundImage.addResolution({1600, 900}, "Textures/SpriteSheetPanel/Background_1600.png"
                                  , {4, 4, 333, 590});
    backgroundImage.addResolution({1280, 720}, "Textures/SpriteSheetPanel/Background_1280.png"
                                  , {3, 3, 266, 472});

    /* Container */
    spriteSheetContainer.setNumColumns(2);
    spriteSheetContainer.setCellWidth(156);
    spriteSheetContainer.setCellHeight(162);

    /* Remove sheet button */
    remSheetButton.normalImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveNormal.png");
    remSheetButton.hoveredImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveHovered.png");
    remSheetButton.pressedImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveNormal.png");
    remSheetButton.disabledImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/RemoveDisabled.png");
    remSheetButton.text.setFont("Fonts/B612-Regular.ttf", 33);
    remSheetButton.text.setText("");
    remSheetButton.disable();

    // Add a callback to remove a selected component on button press.
    remSheetButton.setOnPressed([&](){
        // Set up our data for the confirmation dialog.
        std::string bodyText{"Remove the selected sprite sheet and all associated sprites?"};

        std::function<void(void)> onConfirmation = [&]() {
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
        };

        // Bring up the confirmation dialog.
        mainScreen.openConfirmationDialog(bodyText, "REMOVE", std::move(onConfirmation));
    });

    /* Add sheet button */
    addSheetButton.normalImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/AddNormal.png");
    addSheetButton.hoveredImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/AddHovered.png");
    addSheetButton.pressedImage.addResolution({1920, 1080}, "Textures/SpriteSheetPanel/AddNormal.png");
    addSheetButton.text.setFont("Fonts/B612-Regular.ttf", 33);
    addSheetButton.text.setText("");

    addSheetButton.setOnPressed([this](){
        // Bring up the add dialog.
        addSheetDialog.setIsVisible(true);
    });

    /* Dialog. */
    addSheetDialog.setIsVisible(false);

    // Register for the events that we want to listen for.
    registerListener(AUI::InternalEvent::MouseButtonDown);
}

void SpriteSheetPanel::addSpriteSheet(const SpriteSheet& sheet)
{
    std::unique_ptr<AUI::Component> thumbnailPtr{
        std::make_unique<MainThumbnail>(screen, "")};
    MainThumbnail& thumbnail{static_cast<MainThumbnail&>(*thumbnailPtr)};

    thumbnail.thumbnailImage.addResolution({1280, 720}, sheet.relPath);
    thumbnail.setText(sheet.relPath);
    thumbnail.setIsActivateable(false);

    // Add a callback to deselect all other components when this one
    // is selected.
    thumbnail.setOnSelected([&](AUI::Thumbnail* selectedThumb){
        // Deselect all other thumbnails.
        for (auto& componentPtr : spriteSheetContainer) {
            MainThumbnail& otherThumb = static_cast<MainThumbnail&>(*componentPtr);
            if (otherThumb.getIsSelected() && (&otherThumb != selectedThumb)) {
                otherThumb.deselect();
            }
        }

        // Make sure the remove button is enabled.
        remSheetButton.enable();
    });

    // Add a callback to disable the remove button if nothing is selected.
    thumbnail.setOnDeselected([&](AUI::Thumbnail* deselectedThumb){
        ignore(deselectedThumb);

        // Check if any thumbnails are selected.
        bool thumbIsSelected{false};
        for (auto& componentPtr : spriteSheetContainer) {
            MainThumbnail& otherThumb = static_cast<MainThumbnail&>(*componentPtr);
            if (otherThumb.getIsSelected()) {
                thumbIsSelected = true;
            }
        }

        // If none of the thumbs are selected, disable the remove button.
        if (!thumbIsSelected) {
            remSheetButton.disable();
        }
    });

    spriteSheetContainer.push_back(std::move(thumbnailPtr));
}

void SpriteSheetPanel::clearSpriteSheets()
{
    spriteSheetContainer.clear();
}

bool SpriteSheetPanel::onMouseButtonDown(SDL_MouseButtonEvent& event)
{
//    // If the click event was outside our extent.
//    if (!(containsPoint({event.x, event.y}))) {
//        // Deselect any selected component.
//        for (auto& componentPtr : spriteSheetContainer) {
//            MainThumbnail& thumbnail = static_cast<MainThumbnail&>(*componentPtr);
//            if (thumbnail.getIsSelected()) {
//                thumbnail.deselect();
//                break;
//            }
//        }
//    }

    return false;
}

void SpriteSheetPanel::render(const SDL_Point& parentOffset)
{
    // Keep our scaling up to date.
    refreshScaling();

    // Save the extent that we're going to render at.
    lastRenderedExtent = scaledExtent;
    lastRenderedExtent.x += parentOffset.x;
    lastRenderedExtent.y += parentOffset.y;

    // Children should render at the parent's offset + this component's offset.
    SDL_Point childOffset{parentOffset};
    childOffset.x += scaledExtent.x;
    childOffset.y += scaledExtent.y;

    // Render our children.
    backgroundImage.render(childOffset);

    spriteSheetContainer.render(childOffset);

    remSheetButton.render(childOffset);

    addSheetButton.render(childOffset);

    addSheetDialog.render(childOffset);
}

} // End namespace SpriteEditor
} // End namespace AM
