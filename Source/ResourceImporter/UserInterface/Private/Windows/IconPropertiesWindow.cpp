#include "IconPropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "EditorIcon.h"
#include "Paths.h"
#include <string>
#include <iostream>

namespace AM
{
namespace ResourceImporter
{
IconPropertiesWindow::IconPropertiesWindow(DataModel& inDataModel)
: AUI::Window({1617, 0, 303, 518}, "IconPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, dataModel{inDataModel}
, activeIconID{NULL_ICON_ID}
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
    windowLabel.setText("Icon Properties");

    /* Display name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});
    nameInput.setOnTextCommitted([this]() { saveName(); });

    // When the active icon is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&IconPropertiesWindow::onActiveLibraryItemChanged>(*this);
    dataModel.iconModel.iconDisplayNameChanged
        .connect<&IconPropertiesWindow::onIconDisplayNameChanged>(*this);
    dataModel.iconModel.iconRemoved
        .connect<&IconPropertiesWindow::onIconRemoved>(*this);
}

void IconPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is an icon and return early if not.
    const EditorIcon* newActiveIcon{std::get_if<EditorIcon>(&newActiveItem)};
    if (newActiveIcon == nullptr) {
        activeIconID = NULL_ICON_ID;
        return;
    }

    activeIconID = newActiveIcon->numericID;

    // Update all of our property fields to match the new active icon's data.
    nameInput.setText(newActiveIcon->displayName);
}

void IconPropertiesWindow::onIconRemoved(IconID iconID)
{
    if (iconID == activeIconID) {
        activeIconID = NULL_ICON_ID;
        nameInput.setText("");
    }
}

void IconPropertiesWindow::onIconDisplayNameChanged(
    IconID iconID, const std::string& newDisplayName)
{
    if (iconID == activeIconID) {
        nameInput.setText(newDisplayName);
    }
}

void IconPropertiesWindow::saveName()
{
    dataModel.iconModel.setIconDisplayName(activeIconID, nameInput.getText());
}

} // End namespace ResourceImporter
} // End namespace AM
