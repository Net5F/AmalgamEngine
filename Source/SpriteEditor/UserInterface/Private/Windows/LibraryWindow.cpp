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
, addButton({286, 9, 22, 22}, "AddButton")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(libraryContainer);
    children.push_back(windowLabel);
    children.push_back(addButton);

    // Flag ourselves as focusable, so we can receive keyboard events.
    isFocusable = true;

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
    // Note: These must be ordered to match the Category enum.
    auto sheetContainer{
        std::make_unique<LibraryCollapsibleContainer>("Sprite Sheets")};
    libraryContainer.push_back(std::move(sheetContainer));
    auto floorContainer{
        std::make_unique<LibraryCollapsibleContainer>("Floors")};
    libraryContainer.push_back(std::move(floorContainer));
    auto floorCoveringContainer{
        std::make_unique<LibraryCollapsibleContainer>("Floor Coverings")};
    libraryContainer.push_back(std::move(floorCoveringContainer));
    auto wallContainer{
        std::make_unique<LibraryCollapsibleContainer>("Walls")};
    libraryContainer.push_back(std::move(wallContainer));
    auto objectContainer{
        std::make_unique<LibraryCollapsibleContainer>("Objects")};
    libraryContainer.push_back(std::move(objectContainer));

    /* Add list item button */
    addButton.normalImage.setSimpleImage(Paths::TEXTURE_DIR
                                         + "LibraryWindow/NewIcon.png");
    addButton.hoveredImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "LibraryWindow/NewHoveredIcon.png");
    addButton.pressedImage.setSimpleImage(Paths::TEXTURE_DIR
                                          + "LibraryWindow/NewIcon.png");

    addButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    addButton.text.setText("");

    addButton.setOnPressed([this]() {
        // Bring up the add menu.
        mainScreen.openLibraryAddMenu();
    });

    // When an item is added or removed from the model, update this widget.
    spriteDataModel.sheetAdded.connect<&LibraryWindow::onSheetAdded>(*this);
    spriteDataModel.sheetRemoved.connect<&LibraryWindow::onSheetRemoved>(
        *this);
    spriteDataModel.floorAdded.connect<&LibraryWindow::onFloorAdded>(*this);
    spriteDataModel.floorCoveringAdded
        .connect<&LibraryWindow::onFloorCoveringAdded>(*this);
    spriteDataModel.wallAdded.connect<&LibraryWindow::onWallAdded>(*this);
    spriteDataModel.objectAdded.connect<&LibraryWindow::onObjectAdded>(*this);
    spriteDataModel.spriteSetRemoved
        .connect<&LibraryWindow::onSpriteSetRemoved>(*this);

    // When a sprite's display name is updated, update the matching thumbnail.
    spriteDataModel.spriteDisplayNameChanged
        .connect<&LibraryWindow::onSpriteDisplayNameChanged>(*this);
}

void LibraryWindow::onFocusLost(AUI::FocusLostType focusLostType) 
{
    // Deselect all of our selected list items.
    for (LibraryListItem* listItem : selectedListItems) {
        listItem->deselect();
    }
    selectedListItems.clear();
}

AUI::EventResult LibraryWindow::onKeyDown(SDL_Keycode keyCode)
{
    // If the delete key was pressed, open the confirmation dialog.
    if (keyCode == SDLK_DELETE) {
        // If there aren't any selected list items, return early.
        if (selectedListItems.size() == 0) {
            return AUI::EventResult{.wasHandled{false}};
        }

        // Add the selected items to a vector so any accidental selection 
        // changes don't affect the operation.
        // If any of the selected items aren't removable, return early.
        itemsToRemove.clear();
        for (LibraryListItem* listItem : selectedListItems) {
            if (!isRemovable(listItem->type)) {
                return AUI::EventResult{.wasHandled{false}};
            }
            else {
                itemsToRemove.push_back(listItem);
            }
        }

        // Set up our data for the confirmation dialog.
        std::string endText{(selectedListItems.size() > 1) ? " items?"
                                                           : " item?"};
        std::string bodyText{};
        bodyText += "Delete " + std::to_string(selectedListItems.size())
                    + endText;

        std::function<void(void)> onConfirmation = [&]() {
            for (LibraryListItem* listItem : itemsToRemove) {
                removeListItem(listItem);
            }
            selectedListItems.clear();
        };

        // Bring up the confirmation dialog.
        mainScreen.openConfirmationDialog(bodyText, "DELETE",
                                          std::move(onConfirmation));

        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

void LibraryWindow::onSheetAdded(int sheetID,
                                 const EditorSpriteSheet& sheet)
{
    // Create a container for the new sheet.
    auto sheetListItem{std::make_unique<SpriteSheetListItem>(sheet.relPath)};
    sheetListItem->type = LibraryListItem::Type::SpriteSheet;
    sheetListItem->ID = sheetID;
    listItemMaps[LibraryListItem::Type::SpriteSheet].emplace(
        sheetID, sheetListItem.get());

    sheetListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });

    // Add each of the new sheet's sprites to the sheet container.
    for (int spriteID : sheet.spriteIDs) {
        addSpriteToSheetListItem(*sheetListItem, sheet, spriteID);
    }

    // Add the sheet list item to the sheet container.
    auto& sheetContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::SpriteSheets])};
    sheetContainer.push_back(std::move(sheetListItem));
}

void LibraryWindow::onFloorAdded(Uint16 floorID,
                                 const EditorFloorSpriteSet& floor)
{
    onSpriteSetAdded<EditorFloorSpriteSet>(floorID, floor);
}

void LibraryWindow::onFloorCoveringAdded(
    Uint16 floorCoveringID, const EditorFloorCoveringSpriteSet& floorCovering)
{
    onSpriteSetAdded<EditorFloorCoveringSpriteSet>(floorCoveringID,
                                                   floorCovering);
}

void LibraryWindow::onWallAdded(Uint16 wallID, const EditorWallSpriteSet& wall)
{
    onSpriteSetAdded<EditorWallSpriteSet>(wallID, wall);
}

void LibraryWindow::onObjectAdded(Uint16 objectID,
                                  const EditorObjectSpriteSet& object)
{
    onSpriteSetAdded<EditorObjectSpriteSet>(objectID, object);
}

template<typename T>
void LibraryWindow::onSpriteSetAdded(Uint16 spriteSetID, const T& spriteSet)
{
    // Get the appropriate enum values for the given sprite set type.
    SpriteSet::Type spriteSetType{};
    if constexpr (std::is_same_v<T, EditorFloorSpriteSet>) {
        spriteSetType =  SpriteSet::Type::Floor;
    }
    else if constexpr (std::is_same_v<T, EditorFloorCoveringSpriteSet>) {
        spriteSetType =  SpriteSet::Type::FloorCovering;
    }
    else if constexpr (std::is_same_v<T, EditorWallSpriteSet>) {
        spriteSetType =  SpriteSet::Type::Wall;
    }
    else if constexpr (std::is_same_v<T, EditorObjectSpriteSet>) {
        spriteSetType =  SpriteSet::Type::Object;
    }
    LibraryListItem::Type listItemType{toListItemType(spriteSetType)};
    Category category{toCategory(spriteSetType)};

    // Construct a new list item for this sprite set.
    auto spriteSetListItem{
        std::make_unique<LibraryListItem>(spriteSet.displayName)};
    spriteSetListItem->type = listItemType;
    spriteSetListItem->ID = spriteSetID;
    listItemMaps[listItemType].emplace(spriteSetID, spriteSetListItem.get());

    spriteSetListItem->setLeftPadding(32);

    spriteSetListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    spriteSetListItem->setOnActivated(
        [this, spriteSetType, spriteSetID](LibraryListItem* activatedListItem) {
            AM::ignore(activatedListItem);
            // Set this item's associated sprite set as the active item.
            spriteDataModel.setActiveSpriteSet(spriteSetType, spriteSetID);
        });

    // Add the new list item to the appropriate container.
    auto& listItemContainer{
        static_cast<LibraryCollapsibleContainer&>(*libraryContainer[category])};
    listItemContainer.push_back(std::move(spriteSetListItem));
}

void LibraryWindow::onSheetRemoved(int sheetID)
{
    auto sheetListItemMap{listItemMaps[LibraryListItem::Type::SpriteSheet]};
    auto sheetIt{sheetListItemMap.find(sheetID)};
    if (sheetIt == sheetListItemMap.end()) {
        LOG_FATAL("Failed to find sprite sheet during removal.");
    }

    // Clear any list item selections.
    for (LibraryListItem* listItem : selectedListItems) {
        listItem->deselect();
    }
    selectedListItems.clear();

    // Remove the list item from the container.
    auto& sheetContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::SpriteSheets])};
    sheetContainer.erase(sheetIt->second);

    // Remove the list item from the map.
    sheetListItemMap.erase(sheetIt);
}

void LibraryWindow::onSpriteSetRemoved(SpriteSet::Type type, Uint16 spriteSetID)
{
    auto listItemMap{listItemMaps[toListItemType(type)]};
    auto spriteSetIt{listItemMap.find(spriteSetID)};
    if (spriteSetIt == listItemMap.end()) {
        LOG_FATAL("Failed to find sprite set during removal.");
    }

    // Clear any list item selections.
    for (LibraryListItem* listItem : selectedListItems) {
        listItem->deselect();
    }
    selectedListItems.clear();

    // Remove the list item from the container.
    auto& spriteSetContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[toCategory(type)])};
    spriteSetContainer.erase(spriteSetIt->second);

    // Remove the list item from the map.
    listItemMaps[toListItemType(type)].erase(spriteSetIt);
}

void LibraryWindow::onSpriteDisplayNameChanged(int spriteID,
                                const std::string& newDisplayName)
{
    auto spriteListItemMap{listItemMaps[LibraryListItem::Type::Sprite]};
    auto spriteListItemIt{spriteListItemMap.find(spriteID)};
    if (spriteListItemIt == spriteListItemMap.end()) {
        LOG_FATAL("Failed to find a list item for the given sprite.");
    }

    // Update the list item to use the sprite's new display name.
    LibraryListItem& spriteListItem{*(spriteListItemIt->second)};
    spriteListItem.text.setText(newDisplayName);
}

void LibraryWindow::processSelectedListItem(LibraryListItem* selectedListItem)
{
    // TODO: Currently we only support one selection at a time. When we add 
    //       multi-select, this logic will need to check if the newly selected 
    //       item is compatible with the existing ones.

    // Deselect all of our selected list items.
    for (LibraryListItem* listItem : selectedListItems) {
        listItem->deselect();
    }
    selectedListItems.clear();

    // Add the new item.
    selectedListItems.push_back(selectedListItem);
}

void LibraryWindow::addSpriteToSheetListItem(
    SpriteSheetListItem& sheetListItem,
    const EditorSpriteSheet& sheet, int spriteID)
{
    // Construct a new list item for this sprite.
    const EditorSprite& sprite{spriteDataModel.getSprite(spriteID)};
    auto spriteListItem{std::make_unique<LibraryListItem>(sprite.displayName)};
    spriteListItem->type = LibraryListItem::Type::Sprite;
    spriteListItem->ID = spriteID;
    listItemMaps[LibraryListItem::Type::Sprite].emplace(spriteID,
                                                        spriteListItem.get());

    spriteListItem->setLeftPadding(57);

    spriteListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    spriteListItem->setOnActivated([this, spriteID](LibraryListItem* activatedListItem) {
        AM::ignore(activatedListItem);
        // Set this item's associated sprite as the active item.
        spriteDataModel.setActiveSprite(spriteID);
    });

    // Add the sprite list item to the sheet list item.
    sheetListItem.spriteListItemContainer.push_back(std::move(spriteListItem));
}

bool LibraryWindow::isRemovable(LibraryListItem::Type listItemType)
{
    if (listItemType == LibraryListItem::Type::Sprite) {
        return false;
    }
    else {
        return true;
    }
}

void LibraryWindow::removeListItem(LibraryListItem* listItem)
{
    // Sprites can't be individually removed, return early.
    if (listItem->type == LibraryListItem::Type::Sprite) {
        return;
    }

    switch (listItem->type) {
        case LibraryListItem::Type::SpriteSheet: {
            spriteDataModel.remSpriteSheet(listItem->ID);
            break;
        }
        case LibraryListItem::Type::Floor: {
            spriteDataModel.remFloor(static_cast<Uint16>(listItem->ID));
            break;
        }
        case LibraryListItem::Type::FloorCovering: {
            spriteDataModel.remFloorCovering(static_cast<Uint16>(listItem->ID));
            break;
        }
        case LibraryListItem::Type::Wall: {
            spriteDataModel.remWall(static_cast<Uint16>(listItem->ID));
            break;
        }
        case LibraryListItem::Type::Object: {
            spriteDataModel.remObject(static_cast<Uint16>(listItem->ID));
            break;
        }
        default: {
            LOG_FATAL("Unsupported list item type.");
        }
    }
}

LibraryListItem::Type LibraryWindow::toListItemType(SpriteSet::Type spriteSetType)
{
    switch (spriteSetType) {
        case SpriteSet::Type::Floor: {
            return LibraryListItem::Type::Floor;
        }
        case SpriteSet::Type::FloorCovering: {
            return LibraryListItem::Type::FloorCovering;
        }
        case SpriteSet::Type::Wall: {
            return LibraryListItem::Type::Wall;
        }
        case SpriteSet::Type::Object: {
            return LibraryListItem::Type::Object;
        }
        default: {
            LOG_FATAL("Invalid sprite set type.");
            break;
        }
    }
}

LibraryWindow::Category LibraryWindow::toCategory(SpriteSet::Type spriteSetType)
{
    switch (spriteSetType) {
        case SpriteSet::Type::Floor: {
            return Category::Floors;
        }
        case SpriteSet::Type::FloorCovering: {
            return Category::FloorCoverings;
        }
        case SpriteSet::Type::Wall: {
            return Category::Walls;
        }
        case SpriteSet::Type::Object: {
            return Category::Objects;
        }
        default: {
            LOG_FATAL("Invalid sprite set type.");
            break;
        }
    }
}

} // End namespace SpriteEditor
} // End namespace AM
