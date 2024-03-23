#include "EntityGraphicSetPropertiesWindow.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "DataModel.h"
#include "EditorFloorGraphicSet.h"
#include "SpriteID.h"
#include "Paths.h"
#include "Camera.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include <string>

namespace AM
{
namespace ResourceImporter
{
EntityGraphicSetPropertiesWindow::EntityGraphicSetPropertiesWindow(DataModel& inDataModel)
: AUI::Window({1617, 0, 303, 518}, "EntityGraphicSetPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, dataModel{inDataModel}
, activeGraphicSetID{SDL_MAX_UINT16}
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
    windowLabel.setText("Entity Properties");

    /* Display name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});
    nameInput.setOnTextCommitted([this]() { saveName(); });

    // When the active graphic set is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&EntityGraphicSetPropertiesWindow::onActiveLibraryItemChanged>(*this);
    dataModel.entityGraphicSetModel.entityRemoved
        .connect<&EntityGraphicSetPropertiesWindow::onEntityRemoved>(*this);
    dataModel.entityGraphicSetModel.entityDisplayNameChanged
        .connect<&EntityGraphicSetPropertiesWindow::onEntityDisplayNameChanged>(
            *this);
}

void EntityGraphicSetPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is an entity graphic set and return early if 
    // not.
    const EditorEntityGraphicSet* newActiveGraphicSet{
        get_if<EditorEntityGraphicSet>(&newActiveItem)};
    if (!newActiveGraphicSet) {
        activeGraphicSetID = SDL_MAX_UINT16;
        nameInput.setText("");
        return;
    }

    activeGraphicSetID = newActiveGraphicSet->numericID;

    // Update all of our property fields to match the new active graphic 
    // set's data.
    nameInput.setText(newActiveGraphicSet->displayName);
}

void EntityGraphicSetPropertiesWindow::onEntityRemoved(
    EntityGraphicSetID graphicSetID)
{
    if (graphicSetID == activeGraphicSetID) {
        activeGraphicSetID = SDL_MAX_UINT16;
        nameInput.setText("");
    }
}

void EntityGraphicSetPropertiesWindow::onEntityDisplayNameChanged(
    EntityGraphicSetID graphicSetID, const std::string& newDisplayName)
{
    if (graphicSetID == activeGraphicSetID) {
        nameInput.setText(newDisplayName);
    }
}

void EntityGraphicSetPropertiesWindow::saveName()
{
    dataModel.entityGraphicSetModel.setEntityDisplayName(activeGraphicSetID,
                                                         nameInput.getText());
}

} // End namespace ResourceImporter
} // End namespace AM
