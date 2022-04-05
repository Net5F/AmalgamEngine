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
PropertiesPanel::PropertiesPanel(AssetCache& assetCache, MainScreen& inScreen,
                                 SpriteDataModel& inSpriteDataModel)
: AUI::Window({1605, 0, 315, 502}, "PropertiesPanel")
, nameLabel({36, 24, 65, 28}, "NameLabel")
, nameInput(assetCache, {36, 56, 255, 38}, "NameInput")
, hasBoundingBoxLabel({36, 126, 210, 38}, "HasBBLabel")
, hasBoundingBoxInput({269, 134, 22, 22}, "HasBBInput")
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
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, activeSpriteID{SpriteDataModel::INVALID_SPRITE_ID}
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
    children.push_back(hasBoundingBoxLabel);
    children.push_back(hasBoundingBoxInput);
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

    /* Has bounding box entry. */
    hasBoundingBoxLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    hasBoundingBoxLabel.setColor({255, 255, 255, 255});
    hasBoundingBoxLabel.setVerticalAlignment(
        AUI::Text::VerticalAlignment::Center);
    hasBoundingBoxLabel.setText("Has bounding box");

    hasBoundingBoxInput.uncheckedImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "Checkbox/Unchecked.png"));
    hasBoundingBoxInput.checkedImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR + "Checkbox/Checked.png"));
    hasBoundingBoxInput.setOnChecked([this]() { saveHasBoundingBox(); });
    hasBoundingBoxInput.setOnUnchecked([this]() { saveHasBoundingBox(); });

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
    spriteDataModel.activeSpriteChanged.connect<&PropertiesPanel::onActiveSpriteChanged>(*this);
    spriteDataModel.spriteDisplayNameChanged.connect<&PropertiesPanel::onSpriteDisplayNameChanged>(*this);
    spriteDataModel.spriteHasBoundingBoxChanged.connect<&PropertiesPanel::onSpriteHasBoundingBoxChanged>(*this);
    spriteDataModel.spriteModelBoundsChanged.connect<&PropertiesPanel::onSpriteModelBoundsChanged>(*this);
    spriteDataModel.spriteRemoved.connect<&PropertiesPanel::onSpriteRemoved>(*this);
}

void PropertiesPanel::onActiveSpriteChanged(unsigned int newActiveSpriteID,
                                            const Sprite& newActiveSprite)
{
    activeSpriteID = newActiveSpriteID;

    // Update all of our property fields to match the new active sprite's data.
    nameInput.setText(newActiveSprite.displayName);

    if (newActiveSprite.hasBoundingBox) {
        hasBoundingBoxInput.setCurrentState(AUI::Checkbox::State::Checked);
    }
    else {
        hasBoundingBoxInput.setCurrentState(AUI::Checkbox::State::Unchecked);
    }

    minXInput.setText(toRoundedString(newActiveSprite.modelBounds.minX));
    minYInput.setText(toRoundedString(newActiveSprite.modelBounds.minY));
    minZInput.setText(toRoundedString(newActiveSprite.modelBounds.minZ));
    maxXInput.setText(toRoundedString(newActiveSprite.modelBounds.maxX));
    maxYInput.setText(toRoundedString(newActiveSprite.modelBounds.maxY));
    maxZInput.setText(toRoundedString(newActiveSprite.modelBounds.maxZ));
}

void PropertiesPanel::onSpriteRemoved(unsigned int spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = SpriteDataModel::INVALID_SPRITE_ID;
        nameInput.setText("");
        minXInput.setText("");
        minYInput.setText("");
        minZInput.setText("");
        maxXInput.setText("");
        maxYInput.setText("");
        maxZInput.setText("");
    }
}

void PropertiesPanel::onSpriteDisplayNameChanged(unsigned int spriteID, const std::string& newDisplayName)
{
    if (spriteID == activeSpriteID) {
        nameInput.setText(newDisplayName);
    }
}

void PropertiesPanel::onSpriteHasBoundingBoxChanged(unsigned int spriteID, bool newHasBoundingBox)
{
    if (spriteID == activeSpriteID) {
        if (newHasBoundingBox) {
            hasBoundingBoxInput.setCurrentState(AUI::Checkbox::State::Checked);
        }
        else {
            hasBoundingBoxInput.setCurrentState(AUI::Checkbox::State::Unchecked);
        }
    }
}

void PropertiesPanel::onSpriteModelBoundsChanged(unsigned int spriteID, const BoundingBox& newModelBounds)
{
    if (spriteID == activeSpriteID) {
        minXInput.setText(toRoundedString(newModelBounds.minX));
        minYInput.setText(toRoundedString(newModelBounds.minY));
        minZInput.setText(toRoundedString(newModelBounds.minZ));
        maxXInput.setText(toRoundedString(newModelBounds.maxX));
        maxYInput.setText(toRoundedString(newModelBounds.maxY));
        maxZInput.setText(toRoundedString(newModelBounds.maxZ));
    }
}

std::string PropertiesPanel::toRoundedString(float value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

void PropertiesPanel::saveName()
{
    spriteDataModel.setSpriteDisplayName(activeSpriteID, nameInput.getText());
}

void PropertiesPanel::saveHasBoundingBox()
{
    bool hasBoundingBox{(hasBoundingBoxInput.getCurrentState()
                       == AUI::Checkbox::State::Checked)};
    spriteDataModel.setSpriteHasBoundingBox(activeSpriteID, hasBoundingBox);
}

void PropertiesPanel::saveMinX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinX{std::stof(minXInput.getText())};

        // Clamp the value to its bounds.
        BoundingBox newModelBounds{
                spriteDataModel.getSprite(activeSpriteID).modelBounds};
        newModelBounds.minX = std::clamp(newMinX, 0.f, newModelBounds.maxX);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinX));
    }
}

void PropertiesPanel::saveMinY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinY{std::stof(minYInput.getText())};

        // Clamp the value to its bounds.
        BoundingBox newModelBounds{
                spriteDataModel.getSprite(activeSpriteID).modelBounds};
        newModelBounds.minY = std::clamp(newMinY, 0.f, newModelBounds.maxY);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinY));
    }
}

void PropertiesPanel::saveMinZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinZ{std::stof(minZInput.getText())};

        // Clamp the value to its bounds.
        BoundingBox newModelBounds{
                spriteDataModel.getSprite(activeSpriteID).modelBounds};
        newModelBounds.minY = std::clamp(newMinZ, 0.f, newModelBounds.maxZ);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinZ));
    }
}

void PropertiesPanel::saveMaxX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxX{std::stof(maxXInput.getText())};

        // Clamp the value to its bounds.
        const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
        ScreenPoint bottomRightOffset{
            static_cast<float>(activeSprite.textureExtent.w / 2.f),
            static_cast<float>(activeSprite.textureExtent.h
                               - activeSprite.yOffset)};
        float maxXBound{Transforms::screenToWorld(bottomRightOffset, {}).x};

        BoundingBox newModelBounds{activeSprite.modelBounds};
        newModelBounds.maxX = std::clamp(newMaxX, activeSprite.modelBounds.minX,
                             maxXBound);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        // Input was not valid, reset the field to what it was.
        maxXInput.setText(std::to_string(committedMaxX));
    }
}

void PropertiesPanel::saveMaxY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxY{std::stof(maxYInput.getText())};

        // Clamp the value to its bounds.
        const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
        ScreenPoint bottomLeftOffset{
            static_cast<float>(-(activeSprite.textureExtent.w / 2.f)),
            static_cast<float>(activeSprite.textureExtent.h
                               - activeSprite.yOffset)};
        float maxYBound{Transforms::screenToWorld(bottomLeftOffset, {}).y};

        BoundingBox newModelBounds{activeSprite.modelBounds};
        newModelBounds.maxY = std::clamp(newMaxY, activeSprite.modelBounds.minY,
                              maxYBound);

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void PropertiesPanel::saveMaxZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxZ{std::stof(maxZInput.getText())};

        // Clamp the value to its lower bound.
        // Note: We don't clamp to an upper bound cause it's hard to calc
        //       and not very useful. Can add if we ever care to.
        const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
        float minZ{activeSprite.modelBounds.minZ};
        if (newMaxZ < minZ) {
            newMaxZ = minZ;
        }

        BoundingBox newModelBounds{activeSprite.modelBounds};
        newModelBounds.maxZ = newMaxZ;

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

} // End namespace SpriteEditor
} // End namespace AM
