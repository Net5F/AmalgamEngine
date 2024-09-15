#include "SpritePropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "EditorSprite.h"
#include "SpriteID.h"
#include "Paths.h"
#include "Camera.h"
#include "Transforms.h"
#include "SpriteTools.h"
#include "SharedConfig.h"
#include "AUI/ScalingHelpers.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace AM
{
namespace ResourceImporter
{
SpritePropertiesWindow::SpritePropertiesWindow(MainScreen& inScreen,
                                               DataModel& inDataModel,
                                               LibraryWindow& inLibraryWindow)
: AUI::Window({1617, 0, 303, 579}, "SpritePropertiesWindow")
, mainScreen{inScreen}
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
    boundingBoxButton.setOnPressed([&]() { onBoundingBoxButtonPressed(); });

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
    libraryWindow.selectedItemsChanged
        .connect<&SpritePropertiesWindow::onLibrarySelectedItemsChanged>(*this);
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
        setBoundsFieldsEnabled(false);
    }
    else {
        boundingBoxNameLabel.setText("<Custom>");
        boundingBoxButton.text.setText("Save as");
        setBoundsFieldsEnabled(true);
    }

    const BoundingBox& spriteModelBounds{
        newActiveSprite->getModelBounds(dataModel.boundingBoxModel)};
    minXInput.setText(toRoundedString(spriteModelBounds.min.x));
    minYInput.setText(toRoundedString(spriteModelBounds.min.y));
    minZInput.setText(toRoundedString(spriteModelBounds.min.z));
    maxXInput.setText(toRoundedString(spriteModelBounds.max.x));
    maxYInput.setText(toRoundedString(spriteModelBounds.max.y));
    maxZInput.setText(toRoundedString(spriteModelBounds.max.z));

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

    if (newModelBoundsID) {
        const EditorBoundingBox& boundingBox{
            dataModel.boundingBoxModel.getBoundingBox(newModelBoundsID)};
        boundingBoxNameLabel.setText(boundingBox.displayName);
        boundingBoxButton.text.setText("Custom");
        setBoundsFieldsEnabled(false);
    }
    else {
        boundingBoxNameLabel.setText("<Custom>");
        boundingBoxButton.text.setText("Save as");
        setBoundsFieldsEnabled(true);
    }

    const BoundingBox& newModelBounds{
        sprite.getModelBounds(dataModel.boundingBoxModel)};
    minXInput.setText(toRoundedString(newModelBounds.min.x));
    minYInput.setText(toRoundedString(newModelBounds.min.y));
    minZInput.setText(toRoundedString(newModelBounds.min.z));
    maxXInput.setText(toRoundedString(newModelBounds.max.x));
    maxYInput.setText(toRoundedString(newModelBounds.max.y));
    maxZInput.setText(toRoundedString(newModelBounds.max.z));
}

void SpritePropertiesWindow::onSpriteCustomModelBoundsChanged(
    SpriteID spriteID, const BoundingBox& newCustomModelBounds)
{
    if (spriteID == activeSpriteID) {
        const BoundingBox& newModelBounds{newCustomModelBounds};
        minXInput.setText(toRoundedString(newModelBounds.min.x));
        minYInput.setText(toRoundedString(newModelBounds.min.y));
        minZInput.setText(toRoundedString(newModelBounds.min.z));
        maxXInput.setText(toRoundedString(newModelBounds.max.x));
        maxYInput.setText(toRoundedString(newModelBounds.max.y));
        maxZInput.setText(toRoundedString(newModelBounds.max.z));
    }
}

void SpritePropertiesWindow::onLibrarySelectedItemsChanged(
    const std::vector<LibraryListItem*>& selectedItems)
{
    // If there's no active sprite, do nothing.
    if (!activeSpriteID) {
        return;
    }

    // If a new bounding box is selected, allow the user to assign it.
    if ((selectedItems.size() > 0)
        && (selectedItems[0]->type == LibraryListItem::Type::BoundingBox)
        && (selectedItems[0]->text.asString()
            != boundingBoxNameLabel.asString())) {
        boundingBoxButton.text.setText("Assign");
    }
    // If we have a shared bounding box assigned, allow the user to switch 
    // to a custom bounding box.
    else if (dataModel.spriteModel.getSprite(activeSpriteID).modelBoundsID
             != NULL_BOUNDING_BOX_ID) {
        boundingBoxButton.text.setText("Custom");
    }
    else {
        // Custom bounding box and no selection. Allow the user to save it.
        boundingBoxButton.text.setText("Save as");
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

void SpritePropertiesWindow::onBoundingBoxButtonPressed()
{
    SpriteModel& spriteModel{dataModel.spriteModel};

    const std::string buttonText{boundingBoxButton.text.asString()};
    if (buttonText == "Assign") {
        // If a bounding box is selected, assign it to the active animation.
        // Note: This just uses the first selected graphic. Multi-select is 
        //       ignored.
        const auto& selectedListItems{libraryWindow.getSelectedListItems()};
        bool boundingBoxIsSelected{false};
        for (const LibraryListItem* selectedItem : selectedListItems) {
            if (selectedItem->type == LibraryListItem::Type::BoundingBox) {
                boundingBoxIsSelected = true;
                spriteModel.setSpriteModelBoundsID(
                    activeSpriteID, static_cast<BoundingBoxID>(selectedItem->ID));

                break;
            }
        }
    }
    else if (buttonText == "Custom") {
        // If the sprite isn't already using a custom bounding box, set it.
        if (spriteModel.getSprite(activeSpriteID).modelBoundsID
            != NULL_BOUNDING_BOX_ID) {
            spriteModel.setSpriteModelBoundsID(activeSpriteID,
                                               NULL_BOUNDING_BOX_ID);
        }
    }
    else if (buttonText == "Save as") {
        // If the sprite is using a custom bounding box, open the "Save as" 
        // menu.
        const EditorSprite& sprite{spriteModel.getSprite(activeSpriteID)};
        if (sprite.modelBoundsID == NULL_BOUNDING_BOX_ID) {
            mainScreen.openSaveBoundingBoxDialog(
                sprite.customModelBounds, [&](BoundingBoxID newModelBoundsID) {
                    // The save was completed, set the shared bounding box as
                    // this sprite's model bounds.
                    dataModel.spriteModel.setSpriteModelBoundsID(
                        activeSpriteID, newModelBoundsID);
                });
        }
    }
}

void SpritePropertiesWindow::saveName()
{
    dataModel.spriteModel.setSpriteDisplayName(activeSpriteID,
                                               nameInput.getText());
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
        newModelBounds.min.x = std::clamp(newMinX, 0.f, newModelBounds.max.x);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(
            activeSpriteID, BoundingBox(newModelBounds));
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
        newModelBounds.min.y = std::clamp(newMinY, 0.f, newModelBounds.max.y);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(
            activeSpriteID, BoundingBox(newModelBounds));
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
        newModelBounds.min.z = std::clamp(newMinZ, 0.f, newModelBounds.max.z);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(
            activeSpriteID, BoundingBox(newModelBounds));
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
        BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
            activeSprite.textureExtent, activeSprite.stageOrigin)};
        newModelBounds.max.x
            = std::clamp(newMaxX, newModelBounds.min.x, stageWorldExtent.max.x);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(
            activeSpriteID, BoundingBox(newModelBounds));
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
        BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
            activeSprite.textureExtent, activeSprite.stageOrigin)};
        newModelBounds.max.y
            = std::clamp(newMaxY, newModelBounds.min.y, stageWorldExtent.max.y);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(
            activeSpriteID, BoundingBox(newModelBounds));
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
        const EditorSprite& activeSprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        BoundingBox newModelBounds{
            activeSprite.getModelBounds(dataModel.boundingBoxModel)};
        BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
            activeSprite.textureExtent, activeSprite.stageOrigin)};
        newModelBounds.max.z
            = std::clamp(newMaxZ, newModelBounds.min.z, stageWorldExtent.max.z);

        // Apply the new value.
        dataModel.spriteModel.setSpriteCustomModelBounds(
            activeSpriteID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void SpritePropertiesWindow::saveCollisionEnabled()
{
    bool collisionEnabled{(collisionEnabledInput.getCurrentState()
                           == AUI::Checkbox::State::Checked)};
    dataModel.spriteModel.setSpriteCollisionEnabled(activeSpriteID,
                                                    collisionEnabled);
}

} // End namespace ResourceImporter
} // End namespace AM
