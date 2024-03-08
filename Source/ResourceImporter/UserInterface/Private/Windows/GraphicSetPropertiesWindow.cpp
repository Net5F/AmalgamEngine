#include "GraphicSetPropertiesWindow.h"
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
GraphicSetPropertiesWindow::GraphicSetPropertiesWindow(DataModel& inDataModel)
: AUI::Window({1617, 0, 303, 518}, "GraphicSetPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, dataModel{inDataModel}
, activeGraphicSetType{GraphicSet::Type::None}
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
    windowLabel.setText("Floor Properties");

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
        .connect<&GraphicSetPropertiesWindow::onActiveLibraryItemChanged>(*this);
    dataModel.graphicSetModel.graphicSetRemoved
        .connect<&GraphicSetPropertiesWindow::onGraphicSetRemoved>(*this);
    dataModel.graphicSetModel.graphicSetDisplayNameChanged
        .connect<&GraphicSetPropertiesWindow::onGraphicSetDisplayNameChanged>(
            *this);
}

void GraphicSetPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    if (holds_alternative<EditorFloorGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Floor,
                             get<EditorFloorGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorFloorCoveringGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::FloorCovering,
                             get<EditorFloorCoveringGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorWallGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Wall,
                             get<EditorWallGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorObjectGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Object,
                             get<EditorObjectGraphicSet>(newActiveItem));
    }
    else {
        // New active item is not a graphic set. Clear this panel.
        activeGraphicSetType = GraphicSet::Type::None;
        activeGraphicSetID = SDL_MAX_UINT16;
        nameInput.setText("");
    }
}

void GraphicSetPropertiesWindow::onGraphicSetRemoved(GraphicSet::Type type,
                                                   Uint16 graphicSetID)
{
    if ((type == activeGraphicSetType) && (graphicSetID == activeGraphicSetID)) {
        activeGraphicSetType = GraphicSet::Type::None;
        activeGraphicSetID = SDL_MAX_UINT16;
        nameInput.setText("");
    }
}

void GraphicSetPropertiesWindow::onGraphicSetDisplayNameChanged(
    GraphicSet::Type type, Uint16 graphicSetID, const std::string& newDisplayName)
{
    if ((type == activeGraphicSetType) && (graphicSetID == activeGraphicSetID)) {
        nameInput.setText(newDisplayName);
    }
}

template<typename T>
void GraphicSetPropertiesWindow::loadActiveGraphicSet(
    GraphicSet::Type graphicSetType, const T& newActiveGraphicSet)
{
    activeGraphicSetType = graphicSetType;
    activeGraphicSetID = newActiveGraphicSet.numericID;

    switch (activeGraphicSetType) {
        case GraphicSet::Type::Floor: {
            windowLabel.setText("Floor Properties");
            break;
        }
        case GraphicSet::Type::FloorCovering: {
            windowLabel.setText("Floor Covering Properties");
            break;
        }
        case GraphicSet::Type::Wall: {
            windowLabel.setText("Wall Properties");
            break;
        }
        case GraphicSet::Type::Object: {
            windowLabel.setText("Object Properties");
            break;
        }
    }

    // Update all of our property fields to match the new active graphic set's
    // data.
    nameInput.setText(newActiveGraphicSet.displayName);
}

void GraphicSetPropertiesWindow::saveName()
{
    dataModel.graphicSetModel.setGraphicSetDisplayName(
        activeGraphicSetType, activeGraphicSetID, nameInput.getText());
}

} // End namespace ResourceImporter
} // End namespace AM
