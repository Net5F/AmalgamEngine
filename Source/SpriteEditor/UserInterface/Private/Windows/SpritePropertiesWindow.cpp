#include "SpritePropertiesWindow.h"
#include "MainScreen.h"
#include "MainThumbnail.h"
#include "SpriteDataModel.h"
#include "EditorSprite.h"
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
SpritePropertiesWindow::SpritePropertiesWindow(SpriteDataModel& inSpriteDataModel)
: AUI::Window({1617, 0, 303, 518}, "SpritePropertiesWindow")
, nameLabel({24, 52, 65, 28}, "NameLabel")
, nameInput({24, 84, 255, 38}, "NameInput")
, collisionEnabledLabel({24, 160, 210, 27}, "CollisionLabel")
, collisionEnabledInput({257, 162, 22, 22}, "CollisionInput")
, minXLabel({24, 210, 110, 38}, "MinXLabel")
, minXInput({150, 204, 129, 38}, "MinXInput")
, minYLabel({24, 260, 110, 38}, "MinYLabel")
, minYInput({150, 254, 129, 38}, "MinYInput")
, minZLabel({24, 310, 110, 38}, "MinZLabel")
, minZInput({150, 304, 129, 38}, "MinZInput")
, maxXLabel({24, 360, 110, 38}, "MaxXLabel")
, maxXInput({150, 354, 129, 38}, "MaxXInput")
, maxYLabel({24, 410, 110, 38}, "MaxYLabel")
, maxYInput({150, 404, 129, 38}, "MaxYInput")
, maxZLabel({24, 460, 110, 38}, "MaxZLabel")
, maxZInput({150, 454, 129, 38}, "MaxZInput")
, spriteDataModel{inSpriteDataModel}
, activeSpriteID{EMPTY_SPRITE_ID}
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
, backgroundImage({0, 0, 303, 518}, "PropertiesBackground")
, headerImage({0, 0, 303, 40}, "PropertiesHeader")
, windowLabel({12, 0, 115, 40}, "PropertiesWindowLabel")
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(collisionEnabledLabel);
    children.push_back(collisionEnabledInput);
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
    windowLabel.setText("Properties");

    /* Display name entry. */
    nameLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    nameLabel.setColor({255, 255, 255, 255});
    nameLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    nameLabel.setText("Name");

    nameInput.setTextFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    nameInput.setPadding({0, 8, 0, 8});
    nameInput.setOnTextCommitted([this]() { saveName(); });

    /* Collision enabled entry. */
    collisionEnabledLabel.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 21);
    collisionEnabledLabel.setColor({255, 255, 255, 255});
    collisionEnabledLabel.setText("Collision enabled");

    collisionEnabledInput.uncheckedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Checkbox/Unchecked.png");
    collisionEnabledInput.checkedImage.setSimpleImage(Paths::TEXTURE_DIR
                                                    + "Checkbox/Checked.png");
    collisionEnabledInput.setOnChecked([this]() { saveCollisionEnabled(); });
    collisionEnabledInput.setOnUnchecked([this]() { saveCollisionEnabled(); });

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
    spriteDataModel.activeLibraryItemChanged
        .connect<&SpritePropertiesWindow::onActiveLibraryItemChanged>(*this);
    spriteDataModel.spriteDisplayNameChanged
        .connect<&SpritePropertiesWindow::onSpriteDisplayNameChanged>(*this);
    spriteDataModel.spriteCollisionEnabledChanged
        .connect<&SpritePropertiesWindow::onSpriteCollisionEnabledChanged>(*this);
    spriteDataModel.spriteModelBoundsChanged
        .connect<&SpritePropertiesWindow::onSpriteModelBoundsChanged>(*this);
    spriteDataModel.spriteRemoved.connect<&SpritePropertiesWindow::onSpriteRemoved>(
        *this);
}

void SpritePropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite and return early if not.
    const EditorSprite* newActiveSprite{
        std::get_if<EditorSprite>(&newActiveItem)};
    if (newActiveSprite == nullptr) {
        activeSpriteID = EMPTY_SPRITE_ID;
        return;
    }

    activeSpriteID = newActiveSprite->numericID;

    // Update all of our property fields to match the new active sprite's data.
    nameInput.setText(newActiveSprite->displayName);

    if (newActiveSprite->collisionEnabled) {
        collisionEnabledInput.setCurrentState(AUI::Checkbox::State::Checked);
    }
    else {
        collisionEnabledInput.setCurrentState(AUI::Checkbox::State::Unchecked);
    }

    minXInput.setText(toRoundedString(newActiveSprite->modelBounds.minX));
    minYInput.setText(toRoundedString(newActiveSprite->modelBounds.minY));
    minZInput.setText(toRoundedString(newActiveSprite->modelBounds.minZ));
    maxXInput.setText(toRoundedString(newActiveSprite->modelBounds.maxX));
    maxYInput.setText(toRoundedString(newActiveSprite->modelBounds.maxY));
    maxZInput.setText(toRoundedString(newActiveSprite->modelBounds.maxZ));
}

void SpritePropertiesWindow::onSpriteRemoved(int spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = EMPTY_SPRITE_ID;
        nameInput.setText("");
        minXInput.setText("");
        minYInput.setText("");
        minZInput.setText("");
        maxXInput.setText("");
        maxYInput.setText("");
        maxZInput.setText("");
    }
}

void SpritePropertiesWindow::onSpriteDisplayNameChanged(
    int spriteID, const std::string& newDisplayName)
{
    if (spriteID == activeSpriteID) {
        nameInput.setText(newDisplayName);
    }
}

void SpritePropertiesWindow::onSpriteCollisionEnabledChanged(int spriteID,
                                                      bool newCollisionEnabled)
{
    if (spriteID == activeSpriteID) {
        if (newCollisionEnabled) {
            collisionEnabledInput.setCurrentState(
                AUI::Checkbox::State::Checked);
        }
        else {
            collisionEnabledInput.setCurrentState(
                AUI::Checkbox::State::Unchecked);
        }
    }
}

void SpritePropertiesWindow::onSpriteModelBoundsChanged(
    int spriteID, const BoundingBox& newModelBounds)
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

std::string SpritePropertiesWindow::toRoundedString(float value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

void SpritePropertiesWindow::saveName()
{
    spriteDataModel.setSpriteDisplayName(activeSpriteID, nameInput.getText());
}

void SpritePropertiesWindow::saveCollisionEnabled()
{
    bool collisionEnabled{(collisionEnabledInput.getCurrentState()
                           == AUI::Checkbox::State::Checked)};
    spriteDataModel.setSpriteCollisionEnabled(activeSpriteID, collisionEnabled);
}

void SpritePropertiesWindow::saveMinX()
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
        ignore(e);
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinX));
    }
}

void SpritePropertiesWindow::saveMinY()
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
        ignore(e);
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinY));
    }
}

void SpritePropertiesWindow::saveMinZ()
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
        ignore(e);
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinZ));
    }
}

void SpritePropertiesWindow::saveMaxX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxX{std::stof(maxXInput.getText())};

        // Clamp the value to its bounds.
        const EditorSprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};

        BoundingBox newModelBounds{activeSprite.modelBounds};
        newModelBounds.maxX
            = std::clamp(newMaxX, newModelBounds.minX,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        maxXInput.setText(std::to_string(committedMaxX));
    }
}

void SpritePropertiesWindow::saveMaxY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxY{std::stof(maxYInput.getText())};

        // Clamp the value to its bounds.
        const EditorSprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};

        BoundingBox newModelBounds{activeSprite.modelBounds};
        newModelBounds.maxY
            = std::clamp(newMaxY, newModelBounds.minY,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void SpritePropertiesWindow::saveMaxZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxZ{std::stof(maxZInput.getText())};

        // Clamp the value to its lower bound.
        // Note: We don't clamp to an upper bound cause it's hard to calc
        //       and not very useful. Can add if we ever care to.
        const EditorSprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
        float minZ{activeSprite.modelBounds.minZ};
        if (newMaxZ < minZ) {
            newMaxZ = minZ;
        }

        BoundingBox newModelBounds{activeSprite.modelBounds};
        newModelBounds.maxZ = newMaxZ;

        // Apply the new value.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, newModelBounds);
    } catch (std::exception& e) {
        ignore(e);
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

} // End namespace SpriteEditor
} // End namespace AM
