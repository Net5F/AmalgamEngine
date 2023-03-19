#include "LibraryWindow.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
LibraryWindow::LibraryWindow(MainScreen& inScreen,
                             SpriteDataModel& inSpriteDataModel)
: AUI::Window({0, 0, 320, 1080}, "LibraryWindow")
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, backgroundImage({0, 0, 320, 1080})
, headerImage({0, 0, 320, 40})
, categoryContainer({1, 48, 318, (1080 - 48)}, "CategoryContainer")
, newButton({286, 9, 22, 22})
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(categoryContainer);
    children.push_back(newButton);

    /* Background images */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "LibraryWindow/BodyBackground.png"), {1, 1, 1, 1});
    headerImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "LibraryWindow/HeaderBackground.png"),
        {1, 1, 1, 1});

    /* Container */
    categoryContainer.setNumColumns(1);

    /* New list item button */
    newButton.normalImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/NewIcon.png");
    newButton.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/NewHoveredIcon.png");
    newButton.pressedImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/NewIcon.png");

    newButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    newButton.text.setText("");
}

} // End namespace SpriteEditor
} // End namespace AM
