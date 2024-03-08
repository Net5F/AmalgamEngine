#include "SpritePropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "EditorSprite.h"
#include "SpriteID.h"
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
SpritePropertiesWindow::SpritePropertiesWindow(
    DataModel& inDataModel, LibraryWindow& inLibraryWindow)
: AUI::Window({1617, 0, 303, 579}, "SpritePropertiesWindow")
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, boundingBoxLabel{{24, 160, 210, 27}, "BoundingBoxLabel"}
, boundingBoxNameLabel{{24, 193, 178, 21}, "BoundingBoxNameLabel"}
, boundingBoxButton{{207, 186, 72, 26}, "Assign", "BoundingBoxButton"}
, minXLabel{{24, 232, 110, 38}, "MinXLabel"}
, minXInput{{150, 226, 129, 38}, "MinXInput"}
, minYLabel{{24, 282, 110, 38}, "MinYLabel"}
, minYInput{{150, 276, 129, 38}, "MinYInput"}
, minZLabel{{24, 332, 110, 38}, "MinZLabel"}
, minZInput{{150, 326, 129, 38}, "MinZInput"}
, maxXLabel{{24, 382, 110, 38}, "MaxXLabel"}
, maxXInput{{150, 376, 129, 38}, "MaxXInput"}
, maxYLabel{{24, 432, 110, 38}, "MaxYLabel"}
, maxYInput{{150, 426, 129, 38}, "MaxYInput"}
, maxZLabel{{24, 482, 110, 38}, "MaxZLabel"}
, maxZInput{{150, 476, 129, 38}, "MaxZInput"}
, collisionEnabledLabel{{24, 526, 210, 27}, "CollisionLabel"}
, collisionEnabledInput{{257, 528, 22, 22}, "CollisionInput"}
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeSpriteID{NULL_SPRITE_ID}
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
, backgroundImage{{0, 0, 303, 579}, "PropertiesBackground"}
, headerImage{{0, 0, 303, 40}, "PropertiesHeader"}
, windowLabel{{12, 0, 282, 40}, "PropertiesWindowLabel"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(boundingBoxLabel);
    children.push_back(boundingBoxNameLabel);
    children.push_back(boundingBoxButton);
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
    children.push_back(collisionEnabledLabel);
    children.push_back(collisionEnabledInput);

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
    styleLabel(windowLabel, "Sprite Properties", 21);
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

    /* Bounding box selection. */
    styleLabel(boundingBoxLabel, "Bounding Box", 21);
    styleLabel(boundingBoxNameLabel, "", 16);

    boundingBoxButton.text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 14);
    boundingBoxButton.setOnPressed([&]() { saveModelBoundsID(); });

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

    /* Collision enabled entry. */
    styleLabel(collisionEnabledLabel, "Collision enabled", 21);

    collisionEnabledInput.uncheckedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Checkbox/Unchecked.png");
    collisionEnabledInput.checkedImage.setSimpleImage(Paths::TEXTURE_DIR
                                                      + "Checkbox/Checked.png");
    collisionEnabledInput.setOnChecked([this]() { saveCollisionEnabled(); });
    collisionEnabledInput.setOnUnchecked([this]() { saveCollisionEnabled(); });

    // When the active sprite is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpritePropertiesWindow::onActiveLibraryItemChanged>(*this);
    SpriteModel& spriteModel{dataModel.spriteModel};
    spriteModel.spriteDisplayNameChanged
        .connect<&SpritePropertiesWindow::onSpriteDisplayNameChanged>(*this);
    spriteModel.spriteModelBoundsIDChanged
        .connect<&SpritePropertiesWindow::onSpriteModelBoundsIDChanged>(*this);
    spriteModel.spriteCustomModelBoundsChanged
        .connect<&SpritePropertiesWindow::onSpriteCustomModelBoundsChanged>(*this);
    spriteModel.spriteRemoved.connect<&SpritePropertiesWindow::onSpriteRemoved>(
        *this);
    spriteModel.spriteCollisionEnabledChanged
        .connect<&SpritePropertiesWindow::onSpriteCollisionEnabledChanged>(
            *this);

    // When a library item is selected, update the preview button.
    libraryWindow.listItemSelected
        .connect<&SpritePropertiesWindow::onLibraryListItemSelected>(*this);
    libraryWindow.listItemDeselected
        .connect<&SpritePropertiesWindow::onLibraryListItemDeselected>(*this);
}

void SpritePropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite and return early if not.
    const EditorSprite* newActiveSprite{get_if<EditorSprite>(&newActiveItem)};
    if (!newActiveSprite) {
        activeSpriteID = NULL_SPRITE_ID;
        return;
    }

    activeSpriteID = newActiveSprite->numericID;

    // Update all of our property fields to match the new active sprite's data.
    nameInput.setText(newActiveSprite->displayName);

    if (newActiveSprite->modelBoundsID) {
        const EditorBoundingBox& boundingBox{
            dataModel.boundingBoxModel.getBoundingBox(
                newActiveSprite->modelBoundsID)};
        boundingBoxNameLabel.setText(boundingBox.displayName);
        boundingBoxButton.text.setText("Custom");
        boundingBoxButton.enable();
        setBoundsFieldsEnabled(false);
    }
    else {
        boundingBoxNameLabel.setText("<Custom>");
        boundingBoxButton.text.setText("Assign");
        boundingBoxButton.disable();
        setBoundsFieldsEnabled(true);
    }

    const BoundingBox& spriteModelBounds{
        newActiveSprite->getModelBounds(dataModel.boundingBoxModel)};
    minXInput.setText(toRoundedString(spriteModelBounds.minX));
    minYInput.setText(toRoundedString(spriteModelBounds.minY));
    minZInput.setText(toRoundedString(spriteModelBounds.minZ));
    maxXInput.setText(toRoundedString(spriteModelBounds.maxX));
    maxYInput.setText(toRoundedString(spriteModelBounds.maxY));
    maxZInput.setText(toRoundedString(spriteModelBounds.maxZ));

    if (newActiveSprite->collisionEnabled) {
        collisionEnabledInput.setCurrentState(AUI::Checkbox::State::Checked);
    }
    else {
        collisionEnabledInput.setCurrentState(AUI::Checkbox::State::Unchecked);
    }
}

void SpritePropertiesWindow::onSpriteRemoved(SpriteID spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = NULL_SPRITE_ID;
        nameInput.setText("");
        boundingBoxNameLabel.setText("");
        minXInput.setText("");
        minYInput.setText("");
        minZInput.setText("");
        maxXInput.setText("");
        maxYInput.setText("");
        maxZInput.setText("");
    }
}

void SpritePropertiesWindow::onSpriteDisplayNameChanged(
    SpriteID spriteID, const std::string& newDisplayName)
{
    if (spriteID == activeSpriteID) {
        nameInput.setText(newDisplayName);
    }
}

void SpritePropertiesWindow::onSpriteCollisionEnabledChanged(
    SpriteID spriteID, bool newCollisionEnabled)
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

void SpritePropertiesWindow::onSpriteModelBoundsIDChanged(
    SpriteID spriteID, BoundingBoxID newModelBoundsID)
{
    // If the sprite isn't active, do nothing.
    if (spriteID != activeSpriteID) {
        return;
    }

    // Whether they're enabled or not, the fields should show the correct bounds.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    const BoundingBox& newModelBounds{
        sprite.getModelBounds(dataModel.boundingBoxModel)};

    if (newModelBoundsID) {
        const EditorBoundingBox& boundingBox{
            dataModel.boundingBoxModel.getBoundingBox(newModelBoundsID)};
        boundingBoxNameLabel.setText(boundingBox.displayName);
        setBoundsFieldsEnabled(false);
    }
    else {
        boundingBoxNameLabel.setText("<Custom>");
        setBoundsFieldsEnabled(true);
    }

    minXInput.setText(toRoundedString(newModelBounds.minX));
    minYInput.setText(toRoundedString(newModelBounds.minY));
    minZInput.setText(toRoundedString(newModelBounds.minZ));
    maxXInput.setText(toRoundedString(newModelBounds.maxX));
    maxYInput.setText(toRoundedString(newModelBounds.maxY));
    maxZInput.setText(toRoundedString(newModelBounds.maxZ));
}

void SpritePropertiesWindow::onSpriteCustomModelBoundsChanged(
    SpriteID spriteID, const BoundingBox& newCustomModelBounds)
{
    if (spriteID == activeSpriteID) {
        minXInput.setText(toRoundedString(newCustomModelBounds.minX));
        minYInput.setText(toRoundedString(newCustomModelBounds.minY));
        minZInput.setText(toRoundedString(newCustomModelBounds.minZ));
        maxXInput.setText(toRoundedString(newCustomModelBounds.maxX));
        maxYInput.setText(toRoundedString(newCustomModelBounds.maxY));
        maxZInput.setText(toRoundedString(newCustomModelBounds.maxZ));
    }
}

void SpritePropertiesWindow::onLibraryListItemSelected(
    const LibraryListItem& selectedItem)
{
    // If there's no active sprite, do nothing.
    if (activeSpriteID == NULL_SPRITE_ID) {
        return;
    }

    // TODO: When we add multi-select, this will need to be updated.
    // If a bounding box is selected, allow the user to assign it.
    if (selectedItem.type == LibraryListItem::Type::BoundingBox) {
        boundingBoxButton.text.setText("Assign");
        boundingBoxButton.enable();
    }
    // If we have a shared bounding box assigned, allow the user to switch 
    // to a custom bounding box.
    else if (dataModel.spriteModel.getSprite(activeSpriteID).modelBoundsID
             != NULL_BOUNDING_BOX_ID) {
        boundingBoxButton.text.setText("Custom");
        boundingBoxButton.enable();
    }
    else {
        // Custom bounding box and no selection. Disable the button.
        boundingBoxButton.text.setText("Assign");
        boundingBoxButton.disable();
    }
}

void SpritePropertiesWindow::onLibraryListItemDeselected(
    const LibraryListItem& deselectedItem)
{
    // If there's no active sprite, do nothing.
    if (activeSpriteID == NULL_SPRITE_ID) {
        return;
    }

    // If we have a shared bounding box assigned, allow the user to switch 
    // to a custom bounding box.
    if (dataModel.spriteModel.getSprite(activeSpriteID).modelBoundsID
             != NULL_BOUNDING_BOX_ID) {
        boundingBoxButton.text.setText("Custom");
        boundingBoxButton.enable();
    }
    else {
        // Custom bounding box. Disable the button.
        boundingBoxButton.text.setText("Assign");
        boundingBoxButton.disable();
    }
}

void SpritePropertiesWindow::setBoundsFieldsEnabled(bool isEnabled)
{
    if (isEnabled) {
        minXInput.enable();
        minYInput.enable();
        minZInput.enable();
        maxXInput.enable();
        maxYInput.enable();
        maxZInput.enable();
    }
    else {
        minXInput.disable();
        minYInput.disable();
        minZInput.disable();
        maxXInput.disable();
        maxYInput.disable();
        maxZInput.disable();
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
    dataModel.spriteModel.setSpriteDisplayName(activeSpriteID,
                                               nameInput.getText());
}

void SpritePropertiesWindow::saveCollisionEnabled()
{
    bool collisionEnabled{(collisionEnabledInput.getCurrentState()
                           == AUI::Checkbox::State::Checked)};
    dataModel.spriteModel.setSpriteCollisionEnabled(activeSpriteID,
                                                    collisionEnabled);
}

void SpritePropertiesWindow::saveModelBoundsID()
{
    SpriteModel& spriteModel{dataModel.spriteModel};

    // If a bounding box is selected, assign it to the active sprite.
    const auto& selectedListItems{libraryWindow.getSelectedListItems()};
    bool boundingBoxIsSelected{false};
    for (const LibraryListItem* selectedItem : selectedListItems) {
        // If this is a sprite, update this slot in the model.
        if (selectedItem->type == LibraryListItem::Type::BoundingBox) {
            boundingBoxIsSelected = true;
            spriteModel.setSpriteModelBoundsID(
                activeSpriteID, static_cast<BoundingBoxID>(selectedItem->ID));
        }
    }

    // If a bounding box isn't selected and the sprite isn't already set to a 
    // custom bounding box, set it.
    if (!boundingBoxIsSelected
        && (spriteModel.getSprite(activeSpriteID).modelBoundsID
            != NULL_BOUNDING_BOX_ID)) {
        spriteModel.setSpriteModelBoundsID(activeSpriteID,
                                           NULL_BOUNDING_BOX_ID);
        boundingBoxButton.text.setText("Assign");
        boundingBoxButton.disable();
    }
}

void SpritePropertiesWindow::saveMinX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinX{std::stof(minXInput.getText())};

        // Clamp the value to its bounds.
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        newModelBounds.minX = std::clamp(newMinX, 0.f, newModelBounds.maxX);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         newModelBounds);
    } catch (std::exception&) {
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
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        newModelBounds.minY = std::clamp(newMinY, 0.f, newModelBounds.maxY);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         newModelBounds);
    } catch (std::exception&) {
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
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        newModelBounds.minY = std::clamp(newMinZ, 0.f, newModelBounds.maxZ);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         newModelBounds);
    } catch (std::exception&) {
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
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        newModelBounds.maxX
            = std::clamp(newMaxX, newModelBounds.minX,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         newModelBounds);
    } catch (std::exception&) {
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
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        newModelBounds.maxY
            = std::clamp(newMaxY, newModelBounds.minY,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         newModelBounds);
    } catch (std::exception&) {
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
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        float minZ{newModelBounds.minZ};
        if (newMaxZ < minZ) {
            newMaxZ = minZ;
        }

        newModelBounds.maxZ = newMaxZ;

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         newModelBounds);
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

} // End namespace ResourceImporter
} // End namespace AM
