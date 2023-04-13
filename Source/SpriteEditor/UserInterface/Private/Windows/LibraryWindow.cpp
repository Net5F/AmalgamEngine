#include "LibraryWindow.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"
#include "MainCollapsibleContainer.h"
#include "LibraryListItem.h"

namespace AM
{
namespace SpriteEditor
{
LibraryWindow::LibraryWindow(MainScreen& inScreen,
                             SpriteDataModel& inSpriteDataModel)
: AUI::Window({0, 0, 320, 1080}, "LibraryWindow")
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, backgroundImage({0, 0, 320, 1080}, "LibraryBackground")
, headerImage({0, 0, 320, 40}, "LibraryHeader")
, windowLabel({12, 0, 80, 40}, "LibraryWindowLabel")
, categoryContainer({1, 40, 318, (1080 - 40 - 1)}, "CategoryContainer")
, newButton({286, 9, 22, 22}, "NewButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(categoryContainer);
    children.push_back(windowLabel);
    children.push_back(newButton);

    /* Window setup */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});
    headerImage.setNineSliceImage((Paths::TEXTURE_DIR + "HeaderBackground.png"),
                                  {1, 1, 1, 1});
    windowLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    windowLabel.setColor({255, 255, 255, 255});
    windowLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    windowLabel.setText("Library");

    /* Container */
    for (std::size_t i = 0; i < 30; ++i) {
        std::string temp{"hi: "};
        temp += std::to_string(i);
        auto collapsible{std::make_unique<MainCollapsibleContainer>(temp)};
        collapsible->setLeftPadding(8);
        auto collapsible2{std::make_unique<MainCollapsibleContainer>(temp)};
        collapsible2->setLeftPadding(32);
        auto listItem{std::make_unique<LibraryListItem>("SpriteName")};
        listItem->setLeftPadding(57);
        collapsible2->push_back(std::move(listItem));
        collapsible->push_back(std::move(collapsible2));
        categoryContainer.push_back(std::move(collapsible));
    }

    /* New list item button */
    newButton.normalImage.setSimpleImage(Paths::TEXTURE_DIR
                                         + "LibraryWindow/NewIcon.png");
    newButton.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "LibraryWindow/NewHoveredIcon.png");
    newButton.pressedImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "LibraryWindow/NewIcon.png");

    newButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    newButton.text.setText("");
}

} // End namespace SpriteEditor
} // End namespace AM
