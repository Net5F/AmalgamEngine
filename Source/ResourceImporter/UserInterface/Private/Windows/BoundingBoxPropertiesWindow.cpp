#include "BoundingBoxPropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "Paths.h"
#include "Camera.h"
#include "Transforms.h"
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
    windowLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    windowLabel.setColor({255, 255, 255, 255});
    windowLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    windowLabel.setText("Sprite Properties");

    /* Display name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});
    nameInput.setOnTextCommitted([this]() { saveName(); });

    /* Minimum X-axis bounds entry. */
    minXLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    minXLabel.setColor({255, 255, 255, 255});
    minXLabel.setText("Min X");

    minXInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    minXInput.setPadding({0, 8, 0, 8});
    minXInput.setOnTextCommitted([this]() { saveMinX(); });

    /* Minimum Y-axis bounds entry. */
    minYLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    minYLabel.setColor({255, 255, 255, 255});
    minYLabel.setText("Min Y");

    minYInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    minYInput.setPadding({0, 8, 0, 8});
    minYInput.setOnTextCommitted([this]() { saveMinY(); });

    /* Minimum Z-axis bounds entry. */
    minZLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    minZLabel.setColor({255, 255, 255, 255});
    minZLabel.setText("Min Z");

    minZInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    minZInput.setPadding({0, 8, 0, 8});
    minZInput.setOnTextCommitted([this]() { saveMinZ(); });

    /* Maximum X-axis bounds entry. */
    maxXLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    maxXLabel.setColor({255, 255, 255, 255});
    maxXLabel.setText("Max X");

    maxXInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    maxXInput.setPadding({0, 8, 0, 8});
    maxXInput.setOnTextCommitted([this]() { saveMaxX(); });

    /* Maximum Y-axis bounds entry. */
    maxYLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    maxYLabel.setColor({255, 255, 255, 255});
    maxYLabel.setText("Max Y");

    maxYInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    maxYInput.setPadding({0, 8, 0, 8});
    maxYInput.setOnTextCommitted([this]() { saveMaxY(); });

    /* Maximum Z-axis bounds entry. */
    maxZLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    maxZLabel.setColor({255, 255, 255, 255});
    maxZLabel.setText("Max Z");

    maxZInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    maxZInput.setPadding({0, 8, 0, 8});
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
        std::get_if<EditorBoundingBox>(&newActiveItem)};
    if (!newActiveBoundingBox) {
        activeBoundingBoxID = NULL_BOUNDING_BOX_ID;
        return;
    }

    activeBoundingBoxID = newActiveBoundingBox->numericID;

    // Update all of our property fields to match the new active bounds data.
    nameInput.setText(newActiveBoundingBox->displayName);

    const BoundingBox& modelBounds{newActiveBoundingBox->modelBounds};
    minXInput.setText(toRoundedString(modelBounds.minX));
    minYInput.setText(toRoundedString(modelBounds.minY));
    minZInput.setText(toRoundedString(modelBounds.minZ));
    maxXInput.setText(toRoundedString(modelBounds.maxX));
    maxYInput.setText(toRoundedString(modelBounds.maxY));
    maxZInput.setText(toRoundedString(modelBounds.maxZ));
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
        minXInput.setText(toRoundedString(newBounds.minX));
        minYInput.setText(toRoundedString(newBounds.minY));
        minZInput.setText(toRoundedString(newBounds.minZ));
        maxXInput.setText(toRoundedString(newBounds.maxX));
        maxYInput.setText(toRoundedString(newBounds.maxY));
        maxZInput.setText(toRoundedString(newBounds.maxZ));
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
        newModelBounds.minX = std::clamp(newMinX, 0.f, newModelBounds.maxX);

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        newModelBounds);
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
        newModelBounds.minY = std::clamp(newMinY, 0.f, newModelBounds.maxY);

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        newModelBounds);
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
        newModelBounds.minY = std::clamp(newMinZ, 0.f, newModelBounds.maxZ);

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        newModelBounds);
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
        newModelBounds.maxX
            = std::clamp(newMaxX, newModelBounds.minX,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        newModelBounds);
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
        newModelBounds.maxY
            = std::clamp(newMaxY, newModelBounds.minY,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        newModelBounds);
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
        float minZ{newModelBounds.minZ};
        if (newMaxZ < minZ) {
            newMaxZ = minZ;
        }

        newModelBounds.maxZ = newMaxZ;

        // Apply the new value.
        dataModel.boundingBoxModel.setBoundingBoxBounds(activeBoundingBoxID,
                                                        newModelBounds);
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

} // End namespace ResourceImporter
} // End namespace AM
