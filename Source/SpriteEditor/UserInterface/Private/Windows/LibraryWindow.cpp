#include "LibraryWindow.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"
#include "LibraryCollapsibleContainer.h"
#include "SpriteSheetListItem.h"
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
, libraryContainer({1, 40, 318, (1080 - 40 - 1)}, "LibraryContainer")
, newButton({286, 9, 22, 22}, "NewButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(libraryContainer);
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
    // Add the collapsible categories.
    libraryContainer.resize(Category::Count);

    auto categoryContainer{
        std::make_unique<LibraryCollapsibleContainer>("Sprite Sheets")};
    libraryContainer[Category::SpriteSheets] = std::move(categoryContainer);

    /* New list item button */
    newButton.normalImage.setSimpleImage(Paths::TEXTURE_DIR
                                         + "LibraryWindow/NewIcon.png");
    newButton.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "LibraryWindow/NewHoveredIcon.png");
    newButton.pressedImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "LibraryWindow/NewIcon.png");

    newButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    newButton.text.setText("");

    newButton.setOnPressed([this]() {
        // Bring up the add dialog.
        mainScreen.openAddSheetDialog();
    });

    // When a sprite sheet is added or removed from the model, update this
    // widget.
    spriteDataModel.sheetAdded.connect<&LibraryWindow::onSheetAdded>(*this);
    spriteDataModel.sheetRemoved.connect<&LibraryWindow::onSheetRemoved>(
        *this);
}

void LibraryWindow::onSheetAdded(unsigned int sheetID,
                                 const SpriteSheet& sheet)
{
    // Create a container for the new sheet.
    std::unique_ptr<AUI::Widget> sheetListItemPtr{
        std::make_unique<SpriteSheetListItem>(sheet.relPath)};
    SpriteSheetListItem& sheetListItem{
        static_cast<SpriteSheetListItem&>(*sheetListItemPtr)};

    sheetListItem.setOnSelected([this](LibraryListItem* selectedListItem) {
        // Deselect any selected list items.
        transformListItems([selectedListItem](LibraryListItem& otherListItem) {
            if (otherListItem.getIsSelected()
                && (&otherListItem != selectedListItem)) {
                otherListItem.deselect();
            }
        });
    });

    // Add each of the new sheet's sprites to the sheet container.
    for (unsigned int spriteID : sheet.spriteIDs) {
        addSpriteToSheetListItem(sheetListItem, sheet, spriteID);
    }

    // Add the sheet container to the sheet category container.
    auto& categoryContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::SpriteSheets])};
    categoryContainer.push_back(std::move(sheetListItemPtr));
}

void LibraryWindow::onSheetRemoved(unsigned int sheetID)
{
    //auto sheetIt{thumbnailMap.find(sheetID)};
    //if (sheetIt == thumbnailMap.end()) {
    //    LOG_FATAL("Failed to find sprite sheet during removal.");
    //}

    //// Remove the thumbnail from the container.
    //spriteSheetContainer.erase(sheetIt->second);

    //// Remove the thumbnail from the map.
    //thumbnailMap.erase(sheetIt);
}

void LibraryWindow::addSpriteToSheetListItem(
    SpriteSheetListItem& sheetListItem,
    const SpriteSheet& sheet, unsigned int spriteID)
{
    // Construct a new list item for this sprite.
    const Sprite& sprite{spriteDataModel.getSprite(spriteID)};
    auto spriteListItem{std::make_unique<LibraryListItem>(sprite.displayName)};

    spriteListItem->setLeftPadding(57);

    spriteListItem->setOnSelected([this, spriteID](LibraryListItem* selectedItem) {
        // Deselect any selected list items.
        transformListItems([selectedItem](LibraryListItem& otherItem) {
            if (otherItem.getIsSelected()
                && (&otherItem != selectedItem)) {
                otherItem.deselect();
            }
        });
    });
    spriteListItem->setOnActivated([this, spriteID](LibraryListItem* activatedItem) {
        // Set this item's associated sprite as the active sprite.
        spriteDataModel.setActiveSprite(spriteID);
    });

    // Add the sprite to the sheet list item.
    sheetListItem.spriteListItemContainer.push_back(std::move(spriteListItem));
}

template<class UnaryOperation>
void LibraryWindow::transformListItems(UnaryOperation unaryOp) 
{
    // Transform all list items in the sprite sheet category.
    // Note: We handle the sprite sheet category separately, since it's the 
    //       only one with 2 levels.
    auto& categoryContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::SpriteSheets])};
    for (auto& sheetListItemPtr : categoryContainer) {
        SpriteSheetListItem& sheetListItem{
            static_cast<SpriteSheetListItem&>(*sheetListItemPtr)};
        unaryOp(sheetListItem);

        for (auto& spriteListItemPtr : sheetListItem.spriteListItemContainer) {
            LibraryListItem& spriteListItem{
                static_cast<LibraryListItem&>(*spriteListItemPtr)};
            unaryOp(spriteListItem);
        }
    }

    // Transform all list items in the other categories.
    // TODO
}

} // End namespace SpriteEditor
} // End namespace AM
