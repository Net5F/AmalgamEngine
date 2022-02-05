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
: AUI::Widget(inScreen, {1617, 0, 303, 440}, "PropertiesPanel")
, nameLabel(inScreen, {24, 24, 65, 28})
, nameInput(assetCache, inScreen, {24, 56, 255, 38})
, hasBoundingBoxLabel(inScreen, {24, 126, 210, 38})
, hasBoundingBoxInput(inScreen, {257, 134, 22, 22})
, minXLabel(inScreen, {24, 176, 110, 38})
, minXInput(assetCache, inScreen, {150, 176, 129, 38})
, minYLabel(inScreen, {24, 226, 110, 38})
, minYInput(assetCache, inScreen, {150, 226, 129, 38})
, minZLabel(inScreen, {24, 276, 110, 38})
, minZInput(assetCache, inScreen, {150, 276, 129, 38})
, maxXLabel(inScreen, {24, 326, 110, 38})
, maxXInput(assetCache, inScreen, {150, 326, 129, 38})
, maxYLabel(inScreen, {24, 376, 110, 38})
, maxYInput(assetCache, inScreen, {150, 376, 129, 38})
, maxZLabel(inScreen, {24, 426, 110, 38})
, maxZInput(assetCache, inScreen, {150, 426, 129, 38})
, mainScreen{inScreen}
, spriteDataModel{inSpriteDataModel}
, activeSprite{nullptr}
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
, backgroundImage(inScreen, {-12, -4, 319, 506})
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
                               + "PropertiesPanel/Background.png"));

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
}

void PropertiesPanel::loadActiveSprite(Sprite* inActiveSprite)
{
    // Set the new active sprite.
    activeSprite = inActiveSprite;

    // Refresh the UI with the newly set sprite's data.
    refresh();
}

void PropertiesPanel::refresh()
{
    if (activeSprite == nullptr) {
        LOG_FATAL("Tried to refresh with nullptr data.");
    }

    // Fill the fields with the activeSprite data.
    nameInput.setText(activeSprite->displayName);
    minXInput.setText(toRoundedString(activeSprite->modelBounds.minX));
    minYInput.setText(toRoundedString(activeSprite->modelBounds.minY));
    minZInput.setText(toRoundedString(activeSprite->modelBounds.minZ));
    maxXInput.setText(toRoundedString(activeSprite->modelBounds.maxX));
    maxYInput.setText(toRoundedString(activeSprite->modelBounds.maxY));
    maxZInput.setText(toRoundedString(activeSprite->modelBounds.maxZ));

    if (activeSprite->hasBoundingBox) {
        hasBoundingBoxInput.setCurrentState(AUI::Checkbox::State::Checked);
    }
    else {
        hasBoundingBoxInput.setCurrentState(AUI::Checkbox::State::Unchecked);
    }
}

void PropertiesPanel::clear()
{
    activeSprite = nullptr;
    nameInput.setText("");
    minXInput.setText("");
    minYInput.setText("");
    minZInput.setText("");
    maxXInput.setText("");
    maxYInput.setText("");
    maxZInput.setText("");
}

std::string PropertiesPanel::toRoundedString(float value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

void PropertiesPanel::saveName()
{
    if (activeSprite != nullptr) {
        // If the name hasn't changed, do nothing.
        std::string displayName{nameInput.getText()};
        if (displayName == activeSprite->displayName) {
            return;
        }

        // Make the display name unique.
        // Note: This really should be done by having a setName() function
        //       either on the sprite or on the SpriteDataModel that does
        //       this check, but this works for now.
        int appendedNum{0};
        while (!(spriteDataModel.displayNameIsUnique(displayName))) {
            displayName = nameInput.getText() + std::to_string(appendedNum);
            appendedNum++;
        }

        // Save the display name.
        // Note: All characters that a user can enter are valid in the display
        //       name string, so we don't need to validate.
        activeSprite->displayName = displayName;

        // Refresh the UI since the name is shown in the activeSprite panel.
        mainScreen.refreshActiveSpriteUi();
    }
}

void PropertiesPanel::saveHasBoundingBox()
{
    if (activeSprite != nullptr) {
        // Toggle hasBoundingBox.
        // (We don't have to refresh the UI since hasBoundingBox is read during
        //  rendering).
        activeSprite->hasBoundingBox = !(activeSprite->hasBoundingBox);
    }
}

void PropertiesPanel::saveMinX()
{
    if (activeSprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMinX{std::stof(minXInput.getText())};

            // Clamp the value to its bounds.
            newMinX = std::clamp(newMinX, 0.f, activeSprite->modelBounds.maxX);

            // The input was valid, save it.
            activeSprite->modelBounds.minX = newMinX;
            committedMinX = newMinX;

            // Refresh the UI to reflect the new value;
            mainScreen.refreshActiveSpriteUi();
        } catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            minXInput.setText(std::to_string(committedMinX));
        }
    }
}

void PropertiesPanel::saveMinY()
{
    if (activeSprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMinY{std::stof(minYInput.getText())};

            // Clamp the value to its bounds.
            newMinY = std::clamp(newMinY, 0.f, activeSprite->modelBounds.maxY);

            // The input was valid, save it.
            activeSprite->modelBounds.minY = newMinY;
            committedMinY = newMinY;

            // Refresh the UI to reflect the new value;
            mainScreen.refreshActiveSpriteUi();
        } catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            minYInput.setText(std::to_string(committedMinY));
        }
    }
}

void PropertiesPanel::saveMinZ()
{
    if (activeSprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMinZ{std::stof(minZInput.getText())};

            // Clamp the value to its bounds.
            newMinZ = std::clamp(newMinZ, 0.f, activeSprite->modelBounds.maxZ);

            // The input was valid, save it.
            activeSprite->modelBounds.minZ = newMinZ;
            committedMinZ = newMinZ;

            // Refresh the UI to reflect the new value;
            mainScreen.refreshActiveSpriteUi();
        } catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            minZInput.setText(std::to_string(committedMinZ));
        }
    }
}

void PropertiesPanel::saveMaxX()
{
    if (activeSprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMaxX{std::stof(maxXInput.getText())};

            // Clamp the value to its bounds.
            ScreenPoint bottomRightOffset{
                static_cast<float>(activeSprite->textureExtent.w / 2.f),
                static_cast<float>(activeSprite->textureExtent.h
                                   - activeSprite->yOffset)};
            float maxXBound{Transforms::screenToWorld(bottomRightOffset, {}).x};

            newMaxX = std::clamp(newMaxX, activeSprite->modelBounds.minX,
                                 maxXBound);

            // The input was valid, save it.
            activeSprite->modelBounds.maxX = newMaxX;
            committedMaxX = newMaxX;

            // Refresh the UI to reflect the new value;
            mainScreen.refreshActiveSpriteUi();
        } catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            maxXInput.setText(std::to_string(committedMaxX));
        }
    }
}

void PropertiesPanel::saveMaxY()
{
    if (activeSprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMaxY{std::stof(maxYInput.getText())};

            // Clamp the value to its bounds.
            ScreenPoint bottomLeftOffset{
                static_cast<float>(-(activeSprite->textureExtent.w / 2.f)),
                static_cast<float>(activeSprite->textureExtent.h
                                   - activeSprite->yOffset)};
            float maxYBound{Transforms::screenToWorld(bottomLeftOffset, {}).y};

            newMaxY = std::clamp(newMaxY, activeSprite->modelBounds.minY,
                                 maxYBound);

            // The input was valid, save it.
            activeSprite->modelBounds.maxY = newMaxY;
            committedMaxY = newMaxY;

            // Refresh the UI to reflect the new value;
            mainScreen.refreshActiveSpriteUi();
        } catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            maxYInput.setText(std::to_string(committedMaxY));
        }
    }
}

void PropertiesPanel::saveMaxZ()
{
    if (activeSprite != nullptr) {
        // Validate the user input as a valid float.
        try {
            // Convert the input string to a float.
            float newMaxZ{std::stof(maxZInput.getText())};

            // Clamp the value to its lower bound.
            // Note: We don't clamp to an upper bound cause it's hard to calc
            //       and not very useful. Can add if we ever care to.
            float minZ = activeSprite->modelBounds.minZ;
            if (newMaxZ < minZ) {
                newMaxZ = minZ;
            }

            // The input was valid, save it.
            activeSprite->modelBounds.maxZ = newMaxZ;
            committedMaxZ = newMaxZ;

            // Refresh the UI to reflect the new value;
            mainScreen.refreshActiveSpriteUi();
        } catch (std::exception& e) {
            // Input was not valid, reset the field to what it was.
            maxZInput.setText(std::to_string(committedMaxZ));
        }
    }
}

} // End namespace SpriteEditor
} // End namespace AM
