#include "FloorPropertiesWindow.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "EditorFloorSpriteSet.h"
#include "EmptySpriteID.h"
#include "Paths.h"
#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Ignore.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace AM
{
namespace SpriteEditor
{
FloorPropertiesWindow::FloorPropertiesWindow(SpriteDataModel& inSpriteDataModel)
: AUI::Window({1617, 0, 303, 518}, "FloorPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, spriteDataModel{inSpriteDataModel}
, activeFloorID{SDL_MAX_UINT16}
, backgroundImage{{0, 0, 303, 518}, "PropertiesBackground"}
, headerImage{{0, 0, 303, 40}, "PropertiesHeader"}
, windowLabel{{12, 0, 282, 40}, "PropertiesWindowLabel"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(nameLabel);
    children.push_back(nameInput);

    /* Window setup */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});
    headerImage.setNineSliceImage((Paths::TEXTURE_DIR + "HeaderBackground.png"),
                                  {1, 1, 1, 1});
    windowLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    windowLabel.setColor({255, 255, 255, 255});
    windowLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    windowLabel.setText("Floor Properties");

    /* Display name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});
    nameInput.setOnTextCommitted([this]() { saveName(); });

    // When the active sprite set is updated, update it in this widget.
    spriteDataModel.activeLibraryItemChanged
        .connect<&FloorPropertiesWindow::onActiveLibraryItemChanged>(*this);
    spriteDataModel.spriteSetRemoved
        .connect<&FloorPropertiesWindow::onSpriteSetRemoved>(*this);
    spriteDataModel.spriteSetDisplayNameChanged
        .connect<&FloorPropertiesWindow::onSpriteSetDisplayNameChanged>(*this);
}

void FloorPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a floor and return early if not.
    const auto* newActiveFloor{
        std::get_if<EditorFloorSpriteSet>(&newActiveItem)};
    if (newActiveFloor == nullptr) {
        activeFloorID = SDL_MAX_UINT16;
        return;
    }

    activeFloorID = newActiveFloor->numericID;

    // Update all of our property fields to match the new active floor's data.
    nameInput.setText(newActiveFloor->displayName);
}

void FloorPropertiesWindow::onSpriteSetRemoved(SpriteSet::Type type,
                                               Uint16 spriteSetID)
{
    if ((type == SpriteSet::Type::Floor) && (spriteSetID == activeFloorID)) {
        activeFloorID = SDL_MAX_UINT16;
        nameInput.setText("");
    }
}

void FloorPropertiesWindow::onSpriteSetDisplayNameChanged(
    SpriteSet::Type type, Uint16 spriteSetID, const std::string& newDisplayName)
{
    if ((type == SpriteSet::Type::Floor) && (spriteSetID == activeFloorID)) {
        nameInput.setText(newDisplayName);
    }
}

void FloorPropertiesWindow::saveName()
{
    spriteDataModel.setSpriteSetDisplayName(SpriteSet::Type::Floor,
                                            activeFloorID, nameInput.getText());
}

} // End namespace SpriteEditor
} // End namespace AM
