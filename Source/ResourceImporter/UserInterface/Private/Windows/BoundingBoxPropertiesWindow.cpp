#include "BoundingBoxPropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "Paths.h"
#include "Camera.h"
#include "Transforms.h"
#include "SpriteTools.h"
#include "SharedConfig.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace AM
{
namespace ResourceImporter
{
BoundingBoxPropertiesWindow::BoundingBoxPropertiesWindow(
    DataModel& inDataModel, const LibraryWindow& inLibraryWindow)
: AUI::Window({1617, 0, 303, 579}, "BoundingBoxPropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, minXLabel{{24, 166, 110, 38}, "MinXLabel"}
, minXInput{{150, 160, 129, 38}, "MinXInput"}
, minYLabel{{24, 216, 110, 38}, "MinYLabel"}
, minYInput{{150, 210, 129, 38}, "MinYInput"}
, minZLabel{{24, 266, 110, 38}, "MinZLabel"}
, minZInput{{150, 260, 129, 38}, "MinZInput"}
, maxXLabel{{24, 316, 110, 38}, "MaxXLabel"}
, maxXInput{{150, 310, 129, 38}, "MaxXInput"}
, maxYLabel{{24, 366, 110, 38}, "MaxYLabel"}
, maxYInput{{150, 360, 129, 38}, "MaxYInput"}
, maxZLabel{{24, 416, 110, 38}, "MaxZLabel"}
, maxZInput{{150, 410, 129, 38}, "MaxZInput"}
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeBoundingBoxID{NULL_BOUNDING_BOX_ID}
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
, backgroundImage{{0, 0, 303, 474}, "PropertiesBackground"}
, headerImage{{0, 0, 303, 40}, "PropertiesHeader"}
, windowLabel{{12, 0, 282, 40}, "PropertiesWindowLabel"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(minXLabel);
    children.push_back(minXInput);
    children.push_back(minYLabel);
    children.push_back(minYInput);
    children.push_back(minZLabel);
    children.push_back(minZInput);
    children.push_back(maxXLabel);
    children.push_back(maxXInput);
    children.push_back(maxYLabel);
    children.push_back(maxYInput);
    children.push_back(maxZLabel);
    children.push_back(maxZInput);

    /* Window setup */
    backgroundImage.setNineSliceImage(
        (Paths::TEXTURE_DIR + "WindowBackground.png"), {1, 1, 1, 1});
    headerImage.setNineSliceImage((Paths::TEXTURE_DIR + "HeaderBackground.png"),
                                  {1, 1, 1, 1});

    auto styleLabel
        = [&](AUI::Text& label, const std::string& text, int fontSize) {
        label.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), fontSize);
        label.setColor({255, 255, 255, 255});
        label.setText(text);
    };
    styleLabel(windowLabel, "Bounding Box Properties", 21);
    windowLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    /* Display name entry. */
    styleLabel(nameLabel, "Name", 21);
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    auto styleTextInput = [&](AUI::TextInput& textInput) {
        textInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
        textInput.setPadding({0, 8, 0, 8});
    };
    styleTextInput(nameInput);
    nameInput.setOnTextCommitted([this]() { saveName(); });

    // Bounds entry labels.
    styleLabel(minXLabel, "Min X", 21);
    styleLabel(minYLabel, "Min Y", 21);
    styleLabel(minZLabel, "Min Z", 21);
    styleLabel(maxXLabel, "Max X", 21);
    styleLabel(maxYLabel, "Max Y", 21);
    styleLabel(maxZLabel, "Max Z", 21);

    // Bounds entry text inputs.
    styleTextInput(minXInput);
    styleTextInput(minYInput);
    styleTextInput(minZInput);
    styleTextInput(maxXInput);
    styleTextInput(maxYInput);
    styleTextInput(maxZInput);
    minXInput.setOnTextCommitted([this]() { saveMinX(); });
    minYInput.setOnTextCommitted([this]() { saveMinY(); });
    minZInput.setOnTextCommitted([this]() { saveMinZ(); });
    maxXInput.setOnTextCommitted([this]() { saveMaxX(); });
    maxYInput.setOnTextCommitted([this]() { saveMaxY(); });
    maxZInput.setOnTextCommitted([this]() { saveMaxZ(); });

    // When the active sprite is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&BoundingBoxPropertiesWindow::onActiveLibraryItemChanged>(*this);
    BoundingBoxModel& boundingBoxModel{dataModel.boundingBoxModel};
    boundingBoxModel.boundingBoxDisplayNameChanged
        .connect<&BoundingBoxPropertiesWindow::onBoundingBoxDisplayNameChanged>(
            *this);
    boundingBoxModel.boundingBoxBoundsChanged
        .connect<&BoundingBoxPropertiesWindow::onBoundingBoxBoundsChanged>(
            *this);
    boundingBoxModel.boundingBoxRemoved
        .connect<&BoundingBoxPropertiesWindow::onBoundingBoxRemoved>(*this);
}

void BoundingBoxPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a bounding box and return early if not.
    const EditorBoundingBox* newActiveBoundingBox{
        get_if<EditorBoundingBox>(&newActiveItem)};
    if (!newActiveBoundingBox) {
        activeBoundingBoxID = NULL_BOUNDING_BOX_ID;
        return;
    }

    activeBoundingBoxID = newActiveBoundingBox->numericID;

    // Update all of our property fields to match the new active bounds data.
    nameInput.setText(newActiveBoundingBox->displayName);

    const BoundingBox& modelBounds{newActiveBoundingBox->modelBounds};
    minXInput.setText(toRoundedString(modelBounds.min.x));
    minYInput.setText(toRoundedString(modelBounds.min.y));
    minZInput.setText(toRoundedString(modelBounds.min.z));
    maxXInput.setText(toRoundedString(modelBounds.max.x));
    maxYInput.setText(toRoundedString(modelBounds.max.y));
    maxZInput.setText(toRoundedString(modelBounds.max.z));
}

void BoundingBoxPropertiesWindow::onBoundingBoxRemoved(
    BoundingBoxID boundingBoxID)
{
    if (boundingBoxID == activeBoundingBoxID) {
        activeBoundingBoxID = NULL_BOUNDING_BOX_ID;
        nameInput.setText("");
        minXInput.setText("");
        minYInput.setText("");
        minZInput.setText("");
        maxXInput.setText("");
        maxYInput.setText("");
        maxZInput.setText("");
    }
}

void BoundingBoxPropertiesWindow::onBoundingBoxDisplayNameChanged(
    BoundingBoxID boundingBoxID, const std::string& newDisplayName)
{
    if (boundingBoxID == activeBoundingBoxID) {
        nameInput.setText(newDisplayName);
    }
}

void BoundingBoxPropertiesWindow::onBoundingBoxBoundsChanged(
    BoundingBoxID boundingBoxID, const BoundingBox& newBounds)
{
    if (boundingBoxID == activeBoundingBoxID) {
        const BoundingBox& newMinMaxBounds{newBounds};
        minXInput.setText(toRoundedString(newMinMaxBounds.min.x));
        minYInput.setText(toRoundedString(newMinMaxBounds.min.y));
        minZInput.setText(toRoundedString(newMinMaxBounds.min.z));
        maxXInput.setText(toRoundedString(newMinMaxBounds.max.x));
        maxYInput.setText(toRoundedString(newMinMaxBounds.max.y));
        maxZInput.setText(toRoundedString(newMinMaxBounds.max.z));
    }
}

std::string BoundingBoxPropertiesWindow::toRoundedString(float value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

void BoundingBoxPropertiesWindow::saveName()
{
    dataModel.boundingBoxModel.setBoundingBoxDisplayName(activeBoundingBoxID,
                                                         nameInput.getText());
}

void BoundingBoxPropertiesWindow::saveMinX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinX{std::stof(minXInput.getText())};

        // Clamp the value to its bounds.
        const EditorBoundingBox& activeBoundingBox{
            dataModel.boundingBoxModel.getBoundingBox(activeBoundingBoxID)};
        BoundingBox newModelBounds{activeBoundingBox.modelBounds};
        newModelBounds.min.x = std::clamp(newMinX, 0.f, newModelBounds.max.x);

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(
            activeBoundingBoxID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinX));
    }
}

void BoundingBoxPropertiesWindow::saveMinY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinY{std::stof(minYInput.getText())};

        // Clamp the value to its bounds.
        const EditorBoundingBox& activeBoundingBox{
            dataModel.boundingBoxModel.getBoundingBox(activeBoundingBoxID)};
        BoundingBox newModelBounds{activeBoundingBox.modelBounds};
        newModelBounds.min.y = std::clamp(newMinY, 0.f, newModelBounds.max.y);

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(
            activeBoundingBoxID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinY));
    }
}

void BoundingBoxPropertiesWindow::saveMinZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinZ{std::stof(minZInput.getText())};

        // Clamp the value to its bounds.
        const EditorBoundingBox& activeBoundingBox{
            dataModel.boundingBoxModel.getBoundingBox(activeBoundingBoxID)};
        BoundingBox newModelBounds{activeBoundingBox.modelBounds};
        newModelBounds.min.z = std::clamp(newMinZ, 0.f, newModelBounds.max.z);

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(
            activeBoundingBoxID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinZ));
    }
}

void BoundingBoxPropertiesWindow::saveMaxX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxX{std::stof(maxXInput.getText())};

        // Clamp the value to its bounds.
        const EditorBoundingBox& activeBoundingBox{
            dataModel.boundingBoxModel.getBoundingBox(activeBoundingBoxID)};
        BoundingBox newModelBounds{activeBoundingBox.modelBounds};
        newModelBounds.max.x
            = std::clamp(newMaxX, newModelBounds.min.x,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(
            activeBoundingBoxID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxXInput.setText(std::to_string(committedMaxX));
    }
}

void BoundingBoxPropertiesWindow::saveMaxY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxY{std::stof(maxYInput.getText())};

        // Clamp the value to its bounds.
        const EditorBoundingBox& activeBoundingBox{
            dataModel.boundingBoxModel.getBoundingBox(activeBoundingBoxID)};
        BoundingBox newModelBounds{activeBoundingBox.modelBounds};
        newModelBounds.max.y
            = std::clamp(newMaxY, newModelBounds.min.y,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(
            activeBoundingBoxID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void BoundingBoxPropertiesWindow::saveMaxZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxZ{std::stof(maxZInput.getText())};

        // Clamp the value to its lower bound.
        // Note: We don't clamp to an upper bound cause it's hard to calc
        //       and not very useful. Can add if we ever care to.
        const EditorBoundingBox& activeBoundingBox{
            dataModel.boundingBoxModel.getBoundingBox(activeBoundingBoxID)};
        BoundingBox newModelBounds{activeBoundingBox.modelBounds};
        newMaxZ = std::max(newMaxZ, newModelBounds.min.z);
        newModelBounds.max.z = newMaxZ;

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(
            activeBoundingBoxID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

} // End namespace ResourceImporter
} // End namespace AM
