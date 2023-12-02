#include "SpriteSetPropertiesWindow.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "DataModel.h"
#include "EditorFloorSpriteSet.h"
#include "NullSpriteID.h"
#include "Paths.h"
#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include <string>

namespace AM
{
namespace ResourceImporter
{
SpriteSetPropertiesWindow::SpriteSetPropertiesWindow(DataModel& inDataModel)
: AUI::Window({1617, 0, 303, 518}, "SpriteSetPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, dataModel{inDataModel}
, activeSpriteSetType{SpriteSet::Type::None}
, activeSpriteSetID{SDL_MAX_UINT16}
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
    dataModel.activeLibraryItemChanged
        .connect<&SpriteSetPropertiesWindow::onActiveLibraryItemChanged>(*this);
    dataModel.spriteSetModel.spriteSetRemoved
        .connect<&SpriteSetPropertiesWindow::onSpriteSetRemoved>(*this);
    dataModel.spriteSetModel.spriteSetDisplayNameChanged
        .connect<&SpriteSetPropertiesWindow::onSpriteSetDisplayNameChanged>(*this);
}

void SpriteSetPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    if (std::holds_alternative<EditorFloorSpriteSet>(newActiveItem)) {
        loadActiveSpriteSet(SpriteSet::Type::Floor,
                            std::get<EditorFloorSpriteSet>(newActiveItem));
    }
    else if (std::holds_alternative<EditorFloorCoveringSpriteSet>(
                 newActiveItem)) {
        loadActiveSpriteSet(
            SpriteSet::Type::FloorCovering,
            std::get<EditorFloorCoveringSpriteSet>(newActiveItem));
    }
    else if (std::holds_alternative<EditorWallSpriteSet>(newActiveItem)) {
        loadActiveSpriteSet(SpriteSet::Type::Wall,
                            std::get<EditorWallSpriteSet>(newActiveItem));
    }
    else if (std::holds_alternative<EditorObjectSpriteSet>(newActiveItem)) {
        loadActiveSpriteSet(SpriteSet::Type::Object,
                            std::get<EditorObjectSpriteSet>(newActiveItem));
    }
    else {
        // New active item is not a sprite set. Clear this panel.
        activeSpriteSetType = SpriteSet::Type::None;
        activeSpriteSetID = SDL_MAX_UINT16;
        nameInput.setText("");
    }
}

void SpriteSetPropertiesWindow::onSpriteSetRemoved(SpriteSet::Type type,
                                               Uint16 spriteSetID)
{
    if ((type == activeSpriteSetType) && (spriteSetID == activeSpriteSetID)) {
        activeSpriteSetType = SpriteSet::Type::None;
        activeSpriteSetID = SDL_MAX_UINT16;
        nameInput.setText("");
    }
}

void SpriteSetPropertiesWindow::onSpriteSetDisplayNameChanged(
    SpriteSet::Type type, Uint16 spriteSetID, const std::string& newDisplayName)
{
    if ((type == activeSpriteSetType) && (spriteSetID == activeSpriteSetID)) {
        nameInput.setText(newDisplayName);
    }
}

template<typename T>
void SpriteSetPropertiesWindow::loadActiveSpriteSet(
    SpriteSet::Type spriteSetType, const T& newActiveSpriteSet)
{
    activeSpriteSetType = spriteSetType;
    activeSpriteSetID = newActiveSpriteSet.numericID;

    switch (activeSpriteSetType) {
        case SpriteSet::Type::Floor: {
            windowLabel.setText("Floor Properties");
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            windowLabel.setText("Floor Covering Properties");
            break;
        }
        case SpriteSet::Type::Wall: {
            windowLabel.setText("Wall Properties");
            break;
        }
        case SpriteSet::Type::Object: {
            windowLabel.setText("Object Properties");
            break;
        }
    }

    // Update all of our property fields to match the new active sprite set's 
    // data.
    nameInput.setText(newActiveSpriteSet.displayName);
}

void SpriteSetPropertiesWindow::saveName()
{
    dataModel.spriteSetModel.setSpriteSetDisplayName(
        activeSpriteSetType, activeSpriteSetID, nameInput.getText());
}

} // End namespace ResourceImporter
} // End namespace AM
