#include "LibraryWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "Paths.h"
#include "LibraryCollapsibleContainer.h"
#include "ParentListItem.h"
#include "LibraryListItem.h"

namespace AM
{
namespace ResourceImporter
{
LibraryWindow::LibraryWindow(MainScreen& inScreen, DataModel& inDataModel)
: AUI::Window({0, 0, 320, 1080}, "LibraryWindow")
, mainScreen{inScreen}
, dataModel{inDataModel}
, backgroundImage({0, 0, 320, 1080}, "LibraryBackground")
, headerImage({0, 0, 320, 40}, "LibraryHeader")
, windowLabel({12, 0, 80, 40}, "LibraryWindowLabel")
, libraryContainer({1, 40, 318, (1080 - 40 - 1)}, "LibraryContainer")
, addButton({286, 9, 22, 22}, "AddButton")
, listItemSelected{listItemSelectedSig}
, listItemDeselected{listItemDeselectedSig}
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
    auto boundingBoxContainer{
        std::make_unique<LibraryCollapsibleContainer>("Bounding Boxes")};
    libraryContainer.push_back(std::move(boundingBoxContainer));
    auto spriteSheetContainer{
        std::make_unique<LibraryCollapsibleContainer>("Sprite Sheets")};
    libraryContainer.push_back(std::move(spriteSheetContainer));
    auto floorContainer{
        std::make_unique<LibraryCollapsibleContainer>("Floors")};
    libraryContainer.push_back(std::move(floorContainer));
    auto floorCoveringContainer{
        std::make_unique<LibraryCollapsibleContainer>("Floor Coverings")};
    libraryContainer.push_back(std::move(floorCoveringContainer));
    auto wallContainer{std::make_unique<LibraryCollapsibleContainer>("Walls")};
    libraryContainer.push_back(std::move(wallContainer));
    auto objectContainer{
        std::make_unique<LibraryCollapsibleContainer>("Objects")};
    libraryContainer.push_back(std::move(objectContainer));
    auto entityContainer{
        std::make_unique<LibraryCollapsibleContainer>("Entities")};
    libraryContainer.push_back(std::move(entityContainer));
    auto iconSheetContainer{
        std::make_unique<LibraryCollapsibleContainer>("Icon Sheets")};
    libraryContainer.push_back(std::move(iconSheetContainer));

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
    BoundingBoxModel& boundingBoxModel{dataModel.boundingBoxModel};
    boundingBoxModel.boundingBoxAdded
        .connect<&LibraryWindow::onBoundingBoxAdded>(*this);
    boundingBoxModel.boundingBoxRemoved
        .connect<&LibraryWindow::onBoundingBoxRemoved>(*this);
    SpriteModel& spriteModel{dataModel.spriteModel};
    spriteModel.sheetAdded.connect<&LibraryWindow::onSpriteSheetAdded>(*this);
    spriteModel.sheetRemoved.connect<&LibraryWindow::onSpriteSheetRemoved>(
        *this);
    SpriteSetModel& spriteSetModel{dataModel.spriteSetModel};
    spriteSetModel.floorAdded.connect<&LibraryWindow::onFloorAdded>(*this);
    spriteSetModel.floorCoveringAdded
        .connect<&LibraryWindow::onFloorCoveringAdded>(*this);
    spriteSetModel.wallAdded.connect<&LibraryWindow::onWallAdded>(*this);
    spriteSetModel.objectAdded.connect<&LibraryWindow::onObjectAdded>(*this);
    spriteSetModel.spriteSetRemoved.connect<&LibraryWindow::onSpriteSetRemoved>(
        *this);
    IconModel& iconModel{dataModel.iconModel};
    iconModel.sheetAdded.connect<&LibraryWindow::onIconSheetAdded>(*this);
    iconModel.sheetRemoved.connect<&LibraryWindow::onIconSheetRemoved>(*this);

    // When a display name is updated, update the matching thumbnail.
    boundingBoxModel.boundingBoxDisplayNameChanged
        .connect<&LibraryWindow::onBoundingBoxDisplayNameChanged>(*this);
    spriteModel.spriteDisplayNameChanged
        .connect<&LibraryWindow::onSpriteDisplayNameChanged>(*this);
    spriteSetModel.spriteSetDisplayNameChanged
        .connect<&LibraryWindow::onSpriteSetDisplayNameChanged>(*this);
    iconModel.iconDisplayNameChanged
        .connect<&LibraryWindow::onIconDisplayNameChanged>(*this);
}

const std::vector<LibraryListItem*>& LibraryWindow::getSelectedListItems() const
{
    return selectedListItems;
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
        bodyText
            += "Delete " + std::to_string(selectedListItems.size()) + endText;

        std::function<void(void)> onConfirmation = [&]() {
            for (LibraryListItem* listItem : itemsToRemove) {
                removeListItem(listItem);
            }
            selectedListItems.clear();
        };

        // Bring up the confirmation dialog.
        mainScreen.openConfirmationDialog(bodyText, "Delete",
                                          std::move(onConfirmation));

        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

void LibraryWindow::onBoundingBoxAdded(BoundingBoxID boundingBoxID,
                                       const EditorBoundingBox& bounds)
{
    // Construct a new list item for this bounding box.
    auto boundingBoxListItem{
        std::make_unique<LibraryListItem>(bounds.displayName)};
    boundingBoxListItem->type = LibraryListItem::Type::BoundingBox;
    boundingBoxListItem->ID = static_cast<int>(boundingBoxID);
    listItemMaps[LibraryListItem::Type::BoundingBox].emplace(
        boundingBoxID, boundingBoxListItem.get());

    boundingBoxListItem->setLeftPadding(32);

    boundingBoxListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    boundingBoxListItem->setOnDeselected(
        [this](LibraryListItem* deselectedListItem) {
        // Note: Deselect is handled in OnSelected and FocusLost.
        listItemDeselectedSig.publish(*deselectedListItem);
    });
    boundingBoxListItem->setOnActivated(
        [this, boundingBoxID](LibraryListItem*) {
            // Set this list item's associated bounding box as the active item.
            dataModel.setActiveBoundingBox(boundingBoxID);
        });

    // Add the new list item to the appropriate container.
    auto& listItemContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::BoundingBoxes])};
    listItemContainer.push_back(std::move(boundingBoxListItem));
}

void LibraryWindow::onSpriteSheetAdded(int sheetID,
                                       const EditorSpriteSheet& sheet)
{
    // Create a container for the new sheet.
    auto sheetListItem{std::make_unique<ParentListItem>(sheet.relPath)};
    sheetListItem->type = LibraryListItem::Type::SpriteSheet;
    sheetListItem->ID = sheetID;
    listItemMaps[LibraryListItem::Type::SpriteSheet].emplace(
        sheetID, sheetListItem.get());

    sheetListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    sheetListItem->setOnDeselected([this](LibraryListItem* deselectedListItem) {
        // Note: Deselect is handled in OnSelected and FocusLost.
        listItemDeselectedSig.publish(*deselectedListItem);
    });

    // Add each of the new sheet's sprites to its child container.
    for (SpriteID spriteID : sheet.spriteIDs) {
        addSpriteToSheetListItem(*sheetListItem, sheet, spriteID);
    }

    // Add the sheet list item to the sheet container.
    auto& sheetContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::SpriteSheets])};
    sheetContainer.push_back(std::move(sheetListItem));
}

void LibraryWindow::onFloorAdded(FloorSpriteSetID floorID,
                                 const EditorFloorSpriteSet& floor)
{
    onSpriteSetAdded<EditorFloorSpriteSet>(floorID, floor);
}

void LibraryWindow::onFloorCoveringAdded(
    FloorCoveringSpriteSetID floorCoveringID,
    const EditorFloorCoveringSpriteSet& floorCovering)
{
    onSpriteSetAdded<EditorFloorCoveringSpriteSet>(floorCoveringID,
                                                   floorCovering);
}

void LibraryWindow::onWallAdded(WallSpriteSetID wallID,
                                const EditorWallSpriteSet& wall)
{
    onSpriteSetAdded<EditorWallSpriteSet>(wallID, wall);
}

void LibraryWindow::onObjectAdded(ObjectSpriteSetID objectID,
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
        spriteSetType = SpriteSet::Type::Floor;
    }
    else if constexpr (std::is_same_v<T, EditorFloorCoveringSpriteSet>) {
        spriteSetType = SpriteSet::Type::FloorCovering;
    }
    else if constexpr (std::is_same_v<T, EditorWallSpriteSet>) {
        spriteSetType = SpriteSet::Type::Wall;
    }
    else if constexpr (std::is_same_v<T, EditorObjectSpriteSet>) {
        spriteSetType = SpriteSet::Type::Object;
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
        [this, spriteSetType, spriteSetID](LibraryListItem*) {
            // Set this list item's associated sprite set as the active item.
            dataModel.setActiveSpriteSet(spriteSetType, spriteSetID);
        });

    // Add the new list item to the appropriate container.
    auto& listItemContainer{
        static_cast<LibraryCollapsibleContainer&>(*libraryContainer[category])};
    listItemContainer.push_back(std::move(spriteSetListItem));
}

void LibraryWindow::onIconSheetAdded(int sheetID, const EditorIconSheet& sheet)
{
    // Create a container for the new sheet.
    auto sheetListItem{std::make_unique<ParentListItem>(sheet.relPath)};
    sheetListItem->type = LibraryListItem::Type::IconSheet;
    sheetListItem->ID = sheetID;
    listItemMaps[LibraryListItem::Type::IconSheet].emplace(sheetID,
                                                           sheetListItem.get());

    sheetListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    sheetListItem->setOnDeselected([this](LibraryListItem* deselectedListItem) {
        // Note: Deselect is handled in OnSelected and FocusLost.
        listItemDeselectedSig.publish(*deselectedListItem);
    });

    // Add each of the new sheet's icons to its child container.
    for (IconID iconID : sheet.iconIDs) {
        addIconToSheetListItem(*sheetListItem, sheet, iconID);
    }

    // Add the sheet list item to the sheet container.
    auto& sheetContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::IconSheets])};
    sheetContainer.push_back(std::move(sheetListItem));
}

void LibraryWindow::onBoundingBoxRemoved(BoundingBoxID boundingBoxID)
{
    auto boundsListItemMap{listItemMaps[LibraryListItem::Type::BoundingBox]};
    auto boundsIt{boundsListItemMap.find(boundingBoxID)};
    if (boundsIt == boundsListItemMap.end()) {
        LOG_FATAL("Failed to find bounding box during removal.");
    }

    // Clear any list item selections.
    for (LibraryListItem* listItem : selectedListItems) {
        listItem->deselect();
    }
    selectedListItems.clear();

    // Remove the list item from the container.
    auto& boundsContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::BoundingBoxes])};
    boundsContainer.erase(boundsIt->second);

    // Remove the list item from the map.
    boundsListItemMap.erase(boundsIt);
}

void LibraryWindow::onSpriteSheetRemoved(int sheetID)
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

void LibraryWindow::onIconSheetRemoved(int sheetID)
{
    auto sheetListItemMap{listItemMaps[LibraryListItem::Type::IconSheet]};
    auto sheetIt{sheetListItemMap.find(sheetID)};
    if (sheetIt == sheetListItemMap.end()) {
        LOG_FATAL("Failed to find icon sheet during removal.");
    }

    // Clear any list item selections.
    for (LibraryListItem* listItem : selectedListItems) {
        listItem->deselect();
    }
    selectedListItems.clear();

    // Remove the list item from the container.
    auto& sheetContainer{static_cast<LibraryCollapsibleContainer&>(
        *libraryContainer[Category::IconSheets])};
    sheetContainer.erase(sheetIt->second);

    // Remove the list item from the map.
    sheetListItemMap.erase(sheetIt);
}

void LibraryWindow::onBoundingBoxDisplayNameChanged(
    BoundingBoxID boundingBoxID, const std::string& newDisplayName)
{
    auto boundsListItemMap{listItemMaps[LibraryListItem::Type::BoundingBox]};
    auto boundsListItemIt{boundsListItemMap.find(boundingBoxID)};
    if (boundsListItemIt == boundsListItemMap.end()) {
        LOG_FATAL("Failed to find a list item for the given bounding box.");
    }

    // Update the list item to use the bounding box's new display name.
    LibraryListItem& boundsListItem{*(boundsListItemIt->second)};
    boundsListItem.text.setText(newDisplayName);
}

void LibraryWindow::onSpriteDisplayNameChanged(
    SpriteID spriteID, const std::string& newDisplayName)
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

void LibraryWindow::onSpriteSetDisplayNameChanged(
    SpriteSet::Type type, Uint16 spriteSetID, const std::string& newDisplayName)
{
    LibraryListItem::Type spriteSetListItemType{toListItemType(type)};
    auto spriteSetListItemMap{listItemMaps[spriteSetListItemType]};
    auto spriteSetListItemIt{spriteSetListItemMap.find(spriteSetID)};
    if (spriteSetListItemIt == spriteSetListItemMap.end()) {
        LOG_FATAL("Failed to find a list item for the given sprite set.");
    }

    // Update the list item to use the sprite set's new display name.
    LibraryListItem& spriteSetListItem{*(spriteSetListItemIt->second)};
    spriteSetListItem.text.setText(newDisplayName);
}

void LibraryWindow::onIconDisplayNameChanged(IconID iconID,
                                             const std::string& newDisplayName)
{
    auto iconListItemMap{listItemMaps[LibraryListItem::Type::Icon]};
    auto iconListItemIt{iconListItemMap.find(iconID)};
    if (iconListItemIt == iconListItemMap.end()) {
        LOG_FATAL("Failed to find a list item for the given icon.");
    }

    // Update the list item to use the icon's new display name.
    LibraryListItem& iconListItem{*(iconListItemIt->second)};
    iconListItem.text.setText(newDisplayName);
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

    // Signal that an item was selected.
    listItemSelectedSig.publish(*selectedListItem);
}

void LibraryWindow::addSpriteToSheetListItem(ParentListItem& sheetListItem,
                                             const EditorSpriteSheet& sheet,
                                             SpriteID spriteID)
{
    // Construct a new list item for this sprite.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    auto spriteListItem{std::make_unique<LibraryListItem>(sprite.displayName)};
    spriteListItem->type = LibraryListItem::Type::Sprite;
    spriteListItem->ID = spriteID;
    listItemMaps[LibraryListItem::Type::Sprite].emplace(spriteID,
                                                        spriteListItem.get());

    spriteListItem->setLeftPadding(57);

    spriteListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    spriteListItem->setOnDeselected([this](LibraryListItem* deselectedListItem) {
        // Note: Deselect is handled in OnSelected and FocusLost.
        listItemDeselectedSig.publish(*deselectedListItem);
    });
    spriteListItem->setOnActivated(
        [this, spriteID](LibraryListItem* activatedListItem) {
            // Set this item's associated sprite as the active item.
            dataModel.setActiveSprite(spriteID);
        });

    // Add the sprite list item to the sheet list item.
    sheetListItem.childListItemContainer.push_back(std::move(spriteListItem));
}

void LibraryWindow::addIconToSheetListItem(ParentListItem& sheetListItem,
                                           const EditorIconSheet& sheet,
                                           IconID iconID)
{
    // Construct a new list item for this icon.
    const EditorIcon& icon{dataModel.iconModel.getIcon(iconID)};
    auto iconListItem{std::make_unique<LibraryListItem>(icon.displayName)};
    iconListItem->type = LibraryListItem::Type::Icon;
    iconListItem->ID = iconID;
    listItemMaps[LibraryListItem::Type::Icon].emplace(iconID,
                                                      iconListItem.get());

    iconListItem->setLeftPadding(57);

    iconListItem->setOnSelected([this](LibraryListItem* selectedListItem) {
        processSelectedListItem(selectedListItem);
    });
    iconListItem->setOnActivated(
        [this, iconID](LibraryListItem* activatedListItem) {
            // Set this item's associated icon as the active item.
            dataModel.setActiveIcon(iconID);
        });

    // Add the icon list item to the sheet list item.
    sheetListItem.childListItemContainer.push_back(std::move(iconListItem));
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

    SpriteSetModel& spriteSetModel{dataModel.spriteSetModel};
    switch (listItem->type) {
        case LibraryListItem::Type::SpriteSheet: {
            dataModel.spriteModel.remSpriteSheet(listItem->ID);
            break;
        }
        case LibraryListItem::Type::Floor: {
            spriteSetModel.remFloor(
                static_cast<FloorSpriteSetID>(listItem->ID));
            break;
        }
        case LibraryListItem::Type::FloorCovering: {
            spriteSetModel.remFloorCovering(
                static_cast<FloorCoveringSpriteSetID>(listItem->ID));
            break;
        }
        case LibraryListItem::Type::Wall: {
            spriteSetModel.remWall(static_cast<WallSpriteSetID>(listItem->ID));
            break;
        }
        case LibraryListItem::Type::Object: {
            spriteSetModel.remObject(
                static_cast<ObjectSpriteSetID>(listItem->ID));
            break;
        }
        default: {
            LOG_FATAL("Unsupported list item type.");
        }
    }
}

LibraryListItem::Type
    LibraryWindow::toListItemType(SpriteSet::Type spriteSetType)
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

    return LibraryListItem::Type::None;
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

    return LibraryWindow::Category::None;
}

} // End namespace ResourceImporter
} // End namespace AM
