#include "IconEditView.h"
#include "MainScreen.h"
#include "EditorIcon.h"
#include "DataModel.h"
#include "Paths.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
IconEditView::IconEditView(DataModel& inDataModel)
: AUI::Window({320, 58, 1297, 1022}, "IconEditView")
, dataModel{inDataModel}
, activeIconID{NULL_ICON_ID}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, iconImage{{0, 0, 100, 100}, "IconImage"}
, descText{{24, 806, 1240, 100}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(iconImage);
    children.push_back(descText);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Icon");

    styleText(descText);
    descText.setText("Icons are used for items and equipment. They can also be "
                     "used by your project for things like skills.");

    /* Active icon and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditView/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    iconImage.setIsVisible(false);

    // When the active sprite is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&IconEditView::onActiveLibraryItemChanged>(*this);
    dataModel.iconModel.iconRemoved.connect<&IconEditView::onIconRemoved>(
        *this);
}

void IconEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a icon and return early if not.
    const EditorIcon* newActiveIcon{get_if<EditorIcon>(&newActiveItem)};
    if (!newActiveIcon) {
        activeIconID = NULL_ICON_ID;
        return;
    }

    activeIconID = newActiveIcon->numericID;

    // Load the icon's image.
    std::string imagePath{dataModel.getWorkingTexturesDir()};
    imagePath += newActiveIcon->parentIconSheetPath;
    iconImage.setSimpleImage(imagePath, newActiveIcon->textureExtent);

    // Center the icon to the stage's X, but use a fixed Y.
    SDL_Rect centeredIconExtent{newActiveIcon->textureExtent};
    centeredIconExtent.x = logicalExtent.w / 2;
    centeredIconExtent.x -= (centeredIconExtent.w / 2);
    centeredIconExtent.y = AUI::ScalingHelpers::logicalToActual(212);

    // Size the icon image to the icon extent size.
    iconImage.setLogicalExtent(centeredIconExtent);

    // Set the background to the size of the icon.
    checkerboardImage.setLogicalExtent(iconImage.getLogicalExtent());

    // Set the icon and background to be visible.
    checkerboardImage.setIsVisible(true);
    iconImage.setIsVisible(true);
}

void IconEditView::onIconRemoved(IconID iconID)
{
    if (iconID == activeIconID) {
        activeIconID = NULL_ICON_ID;

        // Set everything back to being invisible.
        checkerboardImage.setIsVisible(false);
        iconImage.setIsVisible(false);
    }
}

void IconEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

} // End namespace ResourceImporter
} // End namespace AM
