#include "PropertiesPanel.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "Sprite.h"
#include "AssetCache.h"
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
PropertiesPanel::PropertiesPanel(AssetCache& assetCache,
                                 SpriteDataModel& inSpriteDataModel)
: AUI::Window({1605, 0, 315, 502}, "PropertiesPanel")
, nameLabel({36, 24, 65, 28}, "NameLabel")
, nameInput(assetCache, {36, 56, 255, 38}, "NameInput")
, boxesLabel({36, 126, 110, 38}, "BoxesLabel")
, remBoxButton({162, 126, 39, 39}, "RemBoxButton")
, boxCountLabel({212, 127, 32, 38}, "BoxCountLabel")
, addBoxButton({252, 126, 39, 39}, "AddBoxButton")
, minXLabel({36, 176, 110, 38}, "MinXLabel")
, minXInput(assetCache, {162, 176, 129, 38}, "MinXInput")
, minYLabel({36, 226, 110, 38}, "MinYLabel")
, minYInput(assetCache, {162, 226, 129, 38}, "MinYInput")
, minZLabel({36, 276, 110, 38}, "MinZLabel")
, minZInput(assetCache, {162, 276, 129, 38}, "MinZInput")
, maxXLabel({36, 326, 110, 38}, "MaxXLabel")
, maxXInput(assetCache, {162, 326, 129, 38}, "MaxXInput")
, maxYLabel({36, 376, 110, 38}, "MaxYLabel")
, maxYInput(assetCache, {162, 376, 129, 38}, "MaxYInput")
, maxZLabel({36, 426, 110, 38}, "MaxZLabel")
, maxZInput(assetCache, {162, 426, 129, 38}, "MaxZInput")
, spriteDataModel{inSpriteDataModel}
, activeSpriteID{SpriteDataModel::INVALID_SPRITE_ID}
, activeModelBoundsIndex{SpriteDataModel::INVALID_MODEL_BOUNDS_INDEX}
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
, backgroundImage({0, 0, 315, 502}, "BackgroundImage")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(boxesLabel);
    children.push_back(boxCountLabel);
    children.push_back(addBoxButton);
    children.push_back(remBoxButton);
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

    /* Background image */
    backgroundImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/Background.png"),
        {0, 4, 315, 502});

    /* Display name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setMargins({8, 0, 8, 0});
    nameInput.setOnTextCommitted([this]() { saveName(); });

    /* Bounding box count entry. */
    // Boxes label.
    boxesLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    boxesLabel.setColor({255, 255, 255, 255});
    boxesLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    boxesLabel.setText("Boxes");

    // Remove bounding box button
    remBoxButton.normalImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/RemoveNormal.png"));
    remBoxButton.hoveredImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/RemoveHovered.png"));
    remBoxButton.pressedImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/RemoveNormal.png"));
    remBoxButton.disabledImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/RemoveDisabled.png"));
    remBoxButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    remBoxButton.text.setText("");
    remBoxButton.setOnPressed([this]() { saveRemoveBoundingBox(); });

    // Bounding box count label.
    boxCountLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    boxCountLabel.setColor({255, 255, 255, 255});
    boxCountLabel.setHorizontalAlignment(
        AUI::Text::HorizontalAlignment::Center);
    boxCountLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    boxCountLabel.setText("0");

    // Add bounding box button
    addBoxButton.normalImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/AddNormal.png"));
    addBoxButton.hoveredImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/AddHovered.png"));
    addBoxButton.pressedImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/AddNormal.png"));
    addBoxButton.disabledImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "PropertiesPanel/AddDisabled.png"));
    addBoxButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    addBoxButton.text.setText("");
    addBoxButton.setOnPressed([this]() { saveAddBoundingBox(); });

    /* Minimum X-axis bounds entry. */
    minXLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    minXLabel.setColor({255, 255, 255, 255});
    minXLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minXLabel.setText("Min X");

    minXInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    minXInput.setMargins({8, 0, 8, 0});
    minXInput.setOnTextCommitted([this]() { saveMinX(); });

    /* Minimum Y-axis bounds entry. */
    minYLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    minYLabel.setColor({255, 255, 255, 255});
    minYLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minYLabel.setText("Min Y");

    minYInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    minYInput.setMargins({8, 0, 8, 0});
    minYInput.setOnTextCommitted([this]() { saveMinY(); });

    /* Minimum Z-axis bounds entry. */
    minZLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    minZLabel.setColor({255, 255, 255, 255});
    minZLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    minZLabel.setText("Min Z");

    minZInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    minZInput.setMargins({8, 0, 8, 0});
    minZInput.setOnTextCommitted([this]() { saveMinZ(); });

    /* Maximum X-axis bounds entry. */
    maxXLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    maxXLabel.setColor({255, 255, 255, 255});
    maxXLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxXLabel.setText("Max X");

    maxXInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    maxXInput.setMargins({8, 0, 8, 0});
    maxXInput.setOnTextCommitted([this]() { saveMaxX(); });

    /* Maximum Y-axis bounds entry. */
    maxYLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    maxYLabel.setColor({255, 255, 255, 255});
    maxYLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxYLabel.setText("Max Y");

    maxYInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    maxYInput.setMargins({8, 0, 8, 0});
    maxYInput.setOnTextCommitted([this]() { saveMaxY(); });

    /* Maximum Z-axis bounds entry. */
    maxZLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    maxZLabel.setColor({255, 255, 255, 255});
    maxZLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    maxZLabel.setText("Max Z");

    maxZInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    maxZInput.setMargins({8, 0, 8, 0});
    maxZInput.setOnTextCommitted([this]() { saveMaxZ(); });

    // When the active sprite is updated, update it in this widget.
    spriteDataModel.activeSpriteChanged
        .connect<&PropertiesPanel::onActiveSpriteChanged>(*this);
    spriteDataModel.spriteDisplayNameChanged
        .connect<&PropertiesPanel::onSpriteDisplayNameChanged>(*this);
    spriteDataModel.spriteModelBoundsAdded
        .connect<&PropertiesPanel::onSpriteModelBoundsAdded>(*this);
    spriteDataModel.spriteModelBoundsRemoved
        .connect<&PropertiesPanel::onSpriteModelBoundsRemoved>(*this);
    spriteDataModel.spriteModelBoundsChanged
        .connect<&PropertiesPanel::onSpriteModelBoundsChanged>(*this);
    spriteDataModel.activeSpriteModelBoundsChanged
        .connect<&PropertiesPanel::onActiveSpriteModelBoundsChanged>(*this);
    spriteDataModel.spriteRemoved.connect<&PropertiesPanel::onSpriteRemoved>(
        *this);
}

void PropertiesPanel::onActiveSpriteChanged(unsigned int newActiveSpriteID,
                                            unsigned int newActiveModelBoundsIndex,
                                            const Sprite& newActiveSprite)
{
    activeSpriteID = newActiveSpriteID;
    activeModelBoundsIndex = newActiveModelBoundsIndex;

    // Update all of our property fields to match the new active sprite's data.
    nameInput.setText(newActiveSprite.displayName);

    // Set the box count label.
    unsigned int boxCount{0};
    if (newActiveModelBoundsIndex != SpriteDataModel::INVALID_MODEL_BOUNDS_INDEX) {
        boxCount = newActiveModelBoundsIndex + 1;
    }
    boxCountLabel.setText(std::to_string(boxCount));

    // If the active sprite has a bounding box, fill our fields to match.
    // Otherwise, default them to 0.
    BoundingBox modelBounds{};
    if (activeModelBoundsIndex != SpriteDataModel::INVALID_MODEL_BOUNDS_INDEX) {
        modelBounds = newActiveSprite.modelBounds.at(activeModelBoundsIndex);
    }
    minXInput.setText(toRoundedString(modelBounds.minX));
    minYInput.setText(toRoundedString(modelBounds.minY));
    minZInput.setText(toRoundedString(modelBounds.minZ));
    maxXInput.setText(toRoundedString(modelBounds.maxX));
    maxYInput.setText(toRoundedString(modelBounds.maxY));
    maxZInput.setText(toRoundedString(modelBounds.maxZ));
}

void PropertiesPanel::onSpriteDisplayNameChanged(
    unsigned int spriteID, const std::string& newDisplayName)
{
    if (spriteID == activeSpriteID) {
        nameInput.setText(newDisplayName);
    }
}

void PropertiesPanel::onSpriteModelBoundsAdded(
    unsigned int spriteID, unsigned int addedBoundsIndex,
    const BoundingBox& newModelBounds)
{
    unsigned int boxCount{addedBoundsIndex + 1};
    boxCountLabel.setText(std::to_string(boxCount));
}

void PropertiesPanel::onSpriteModelBoundsRemoved(
    unsigned int spriteID, unsigned int removedBoundsIndex)
{
    unsigned int boxCount{0};
    if (removedBoundsIndex != SpriteDataModel::INVALID_MODEL_BOUNDS_INDEX) {
        boxCount = removedBoundsIndex + 1;
    }

    boxCountLabel.setText(std::to_string(boxCount));
}

void PropertiesPanel::onSpriteModelBoundsChanged(
    unsigned int spriteID, unsigned int changedBoundsIndex, const BoundingBox& newModelBounds)
{
    if ((spriteID == activeSpriteID) && (changedBoundsIndex == activeModelBoundsIndex)) {
        minXInput.setText(toRoundedString(newModelBounds.minX));
        minYInput.setText(toRoundedString(newModelBounds.minY));
        minZInput.setText(toRoundedString(newModelBounds.minZ));
        maxXInput.setText(toRoundedString(newModelBounds.maxX));
        maxYInput.setText(toRoundedString(newModelBounds.maxY));
        maxZInput.setText(toRoundedString(newModelBounds.maxZ));
    }
}

void PropertiesPanel::onActiveSpriteModelBoundsChanged(
    unsigned int newActiveModelBoundsIndex, const BoundingBox& newActiveModelBounds)
{
    activeModelBoundsIndex = newActiveModelBoundsIndex;

    minXInput.setText(toRoundedString(newActiveModelBounds.minX));
    minYInput.setText(toRoundedString(newActiveModelBounds.minY));
    minZInput.setText(toRoundedString(newActiveModelBounds.minZ));
    maxXInput.setText(toRoundedString(newActiveModelBounds.maxX));
    maxYInput.setText(toRoundedString(newActiveModelBounds.maxY));
    maxZInput.setText(toRoundedString(newActiveModelBounds.maxZ));
}

void PropertiesPanel::onSpriteRemoved(unsigned int spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = SpriteDataModel::INVALID_SPRITE_ID;
        activeModelBoundsIndex = SpriteDataModel::INVALID_MODEL_BOUNDS_INDEX;
        nameInput.setText("");
        minXInput.setText("");
        minYInput.setText("");
        minZInput.setText("");
        maxXInput.setText("");
        maxYInput.setText("");
        maxZInput.setText("");
    }
}

std::string PropertiesPanel::toRoundedString(float value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

bool PropertiesPanel::boxIsSelected()
{
    bool spriteExists{(activeSpriteID != SpriteDataModel::INVALID_SPRITE_ID)};
    bool boxExists{(activeModelBoundsIndex
                    != SpriteDataModel::INVALID_MODEL_BOUNDS_INDEX)};
    return (spriteExists && boxExists);
}

void PropertiesPanel::saveName()
{
    if (activeSpriteID == SpriteDataModel::INVALID_SPRITE_ID) {
        return;
    }

    spriteDataModel.setSpriteDisplayName(activeSpriteID, nameInput.getText());
}

void PropertiesPanel::saveRemoveBoundingBox()
{
    if (!boxIsSelected()) {
        return;
    }

    // Remove a bounding box off the end of the vector.
    spriteDataModel.removeSpriteModelBounds(activeSpriteID);
}

void PropertiesPanel::saveAddBoundingBox()
{
    if (activeSpriteID == SpriteDataModel::INVALID_SPRITE_ID) {
        return;
    }

    // Add the new bounding box.
    spriteDataModel.addSpriteModelBounds(activeSpriteID,
                                         SpriteDataModel::DEFAULT_BOUNDING_BOX);
}

void PropertiesPanel::saveMinX()
{
    if (!boxIsSelected()) {
        return;
    }

    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinX{std::stof(minXInput.getText())};

        // Clamp the value to its bounds.
        BoundingBox newModelBounds{spriteDataModel.getSprite(activeSpriteID)
                                       .modelBounds.at(activeModelBoundsIndex)};
        newModelBounds.minX = std::clamp(newMinX, 0.f, newModelBounds.maxX);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(
            activeSpriteID, activeModelBoundsIndex, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinX));
    }
}

void PropertiesPanel::saveMinY()
{
    if (!boxIsSelected()) {
        return;
    }

    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinY{std::stof(minYInput.getText())};

        // Clamp the value to its bounds.
        BoundingBox newModelBounds{spriteDataModel.getSprite(activeSpriteID)
                                       .modelBounds.at(activeModelBoundsIndex)};
        newModelBounds.minY = std::clamp(newMinY, 0.f, newModelBounds.maxY);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(
            activeSpriteID, activeModelBoundsIndex, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinY));
    }
}

void PropertiesPanel::saveMinZ()
{
    if (!boxIsSelected()) {
        return;
    }

    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinZ{std::stof(minZInput.getText())};

        // Clamp the value to its bounds.
        BoundingBox newModelBounds{spriteDataModel.getSprite(activeSpriteID)
                                       .modelBounds.at(activeModelBoundsIndex)};
        newModelBounds.minY = std::clamp(newMinZ, 0.f, newModelBounds.maxZ);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(
            activeSpriteID, activeModelBoundsIndex, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinZ));
    }
}

void PropertiesPanel::saveMaxX()
{
    if (!boxIsSelected()) {
        return;
    }

    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxX{std::stof(maxXInput.getText())};

        // Clamp the value to its bounds.
        const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};

        BoundingBox newModelBounds{
            activeSprite.modelBounds.at(activeModelBoundsIndex)};
        newModelBounds.maxX
            = std::clamp(newMaxX, newModelBounds.minX,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(
            activeSpriteID, activeModelBoundsIndex, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        maxXInput.setText(std::to_string(committedMaxX));
    }
}

void PropertiesPanel::saveMaxY()
{
    if (!boxIsSelected()) {
        return;
    }

    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxY{std::stof(maxYInput.getText())};

        // Clamp the value to its bounds.
        const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};

        BoundingBox newModelBounds{
            activeSprite.modelBounds.at(activeModelBoundsIndex)};
        newModelBounds.maxY
            = std::clamp(newMaxY, newModelBounds.minY,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(
            activeSpriteID, activeModelBoundsIndex, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void PropertiesPanel::saveMaxZ()
{
    if (!boxIsSelected()) {
        return;
    }

    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxZ{std::stof(maxZInput.getText())};

        // Clamp the value to its lower bound.
        // Note: We don't clamp to an upper bound cause it's hard to calc
        //       and not very useful. Can add if we ever care to.
        const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
        float minZ{activeSprite.modelBounds.at(activeModelBoundsIndex).minZ};
        if (newMaxZ < minZ) {
            newMaxZ = minZ;
        }

        BoundingBox newModelBounds{
            activeSprite.modelBounds.at(activeModelBoundsIndex)};
        newModelBounds.maxZ = newMaxZ;

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(
            activeSpriteID, activeModelBoundsIndex, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

} // End namespace SpriteEditor
} // End namespace AM
