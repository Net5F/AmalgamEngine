#include "AnimationPropertiesWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "EditorSprite.h"
#include "AnimationID.h"
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
AnimationPropertiesWindow::AnimationPropertiesWindow(MainScreen& inScreen,
    DataModel& inDataModel, LibraryWindow& inLibraryWindow)
: AUI::Window({1617, 0, 303, 705}, "AnimationPropertiesWindow")
, mainScreen{inScreen}
, nameLabel{{24, 52, 65, 28}, "NameLabel"}
, nameInput{{24, 84, 255, 38}, "NameInput"}
, frameCountLabel{{24, 166, 110, 28}, "FrameCountLabel"}
, frameCountInput{{150, 160, 129, 38}, "FrameCountInput"}
, fpsLabel{{24, 216, 110, 28}, "FpsLabel"}
, fpsInput{{150, 210, 129, 38}, "FpsInput"}
, boundingBoxLabel{{24, 286, 210, 27}, "BoundingBoxLabel"}
, boundingBoxNameLabel{{24, 319, 178, 21}, "BoundingBoxNameLabel"}
, boundingBoxButton{{207, 312, 72, 26}, "Assign", "BoundingBoxButton"}
, minXLabel{{24, 358, 110, 38}, "MinXLabel"}
, minXInput{{150, 352, 129, 38}, "MinXInput"}
, minYLabel{{24, 408, 110, 38}, "MinYLabel"}
, minYInput{{150, 402, 129, 38}, "MinYInput"}
, minZLabel{{24, 458, 110, 38}, "MinZLabel"}
, minZInput{{150, 452, 129, 38}, "MinZInput"}
, maxXLabel{{24, 508, 110, 38}, "MaxXLabel"}
, maxXInput{{150, 502, 129, 38}, "MaxXInput"}
, maxYLabel{{24, 558, 110, 38}, "MaxYLabel"}
, maxYInput{{150, 552, 129, 38}, "MaxYInput"}
, maxZLabel{{24, 608, 110, 38}, "MaxZLabel"}
, maxZInput{{150, 602, 129, 38}, "MaxZInput"}
, collisionEnabledLabel{{24, 652, 210, 27}, "CollisionLabel"}
, collisionEnabledInput{{257, 654, 22, 22}, "CollisionInput"}
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeAnimationID{NULL_ANIMATION_ID}
, committedFrameCount{0}
, committedFps{0}
, committedMinX{0.0}
, committedMinY{0.0}
, committedMinZ{0.0}
, committedMaxX{0.0}
, committedMaxY{0.0}
, committedMaxZ{0.0}
, backgroundImage{{0, 0, logicalExtent.w, logicalExtent.h},
                  "PropertiesBackground"}
, headerImage{{0, 0, 303, 40}, "PropertiesHeader"}
, windowLabel{{12, 0, 282, 40}, "PropertiesWindowLabel"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(nameLabel);
    children.push_back(nameInput);
    children.push_back(frameCountLabel);
    children.push_back(frameCountInput);
    children.push_back(fpsLabel);
    children.push_back(fpsInput);
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
    styleLabel(windowLabel, "Animation Properties", 21);
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

    /* Frame count entry. */
    styleLabel(frameCountLabel, "Frames", 21);
    frameCountLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    styleTextInput(frameCountInput);
    frameCountInput.setOnTextCommitted([this]() { saveFrameCount(); });

    /* FPS entry. */
    styleLabel(fpsLabel, "Fps", 21);
    fpsLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    styleTextInput(fpsInput);
    fpsInput.setOnTextCommitted([this]() { saveFps(); });

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

    // When the active animation is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&AnimationPropertiesWindow::onActiveLibraryItemChanged>(*this);
    AnimationModel& animationModel{dataModel.animationModel};
    animationModel.animationDisplayNameChanged
        .connect<&AnimationPropertiesWindow::onAnimationDisplayNameChanged>(*this);
    animationModel.animationFrameCountChanged
        .connect<&AnimationPropertiesWindow::onAnimationFrameCountChanged>(*this);
    animationModel.animationFpsChanged
        .connect<&AnimationPropertiesWindow::onAnimationFpsChanged>(*this);
    animationModel.animationModelBoundsIDChanged
        .connect<&AnimationPropertiesWindow::onAnimationModelBoundsIDChanged>(*this);
    animationModel.animationCustomModelBoundsChanged
        .connect<&AnimationPropertiesWindow::onAnimationCustomModelBoundsChanged>(*this);
    animationModel.animationRemoved.connect<&AnimationPropertiesWindow::onAnimationRemoved>(
        *this);
    animationModel.animationCollisionEnabledChanged
        .connect<&AnimationPropertiesWindow::onAnimationCollisionEnabledChanged>(
            *this);

    // When a library item is selected, update the preview button.
    libraryWindow.selectedItemsChanged
        .connect<&AnimationPropertiesWindow::onLibrarySelectedItemsChanged>(
            *this);
}

void AnimationPropertiesWindow::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is an animation and return early if not.
    const EditorAnimation* newActiveAnimation{
        get_if<EditorAnimation>(&newActiveItem)};
    if (!newActiveAnimation) {
        activeAnimationID = NULL_ANIMATION_ID;
        return;
    }

    activeAnimationID = newActiveAnimation->numericID;

    // Update all of our property fields to match the new active animation's 
    // data.
    nameInput.setText(newActiveAnimation->displayName);
    frameCountInput.setText(std::to_string(newActiveAnimation->frameCount));
    fpsInput.setText(std::to_string(newActiveAnimation->fps));

    if (newActiveAnimation->modelBoundsID) {
        const EditorBoundingBox& boundingBox{
            dataModel.boundingBoxModel.getBoundingBox(
                newActiveAnimation->modelBoundsID)};
        boundingBoxNameLabel.setText(boundingBox.displayName);
        boundingBoxButton.text.setText("Custom");
        setBoundsFieldsEnabled(false);
    }
    else {
        boundingBoxNameLabel.setText("<Custom>");
        boundingBoxButton.text.setText("Save as");
        setBoundsFieldsEnabled(true);
    }

    const BoundingBox& animationModelBounds{
        newActiveAnimation->getModelBounds(dataModel.boundingBoxModel)};
    minXInput.setText(toRoundedString(animationModelBounds.min.x));
    minYInput.setText(toRoundedString(animationModelBounds.min.y));
    minZInput.setText(toRoundedString(animationModelBounds.min.z));
    maxXInput.setText(toRoundedString(animationModelBounds.max.x));
    maxYInput.setText(toRoundedString(animationModelBounds.max.y));
    maxZInput.setText(toRoundedString(animationModelBounds.max.z));

    if (newActiveAnimation->collisionEnabled) {
        collisionEnabledInput.setCurrentState(AUI::Checkbox::State::Checked);
    }
    else {
        collisionEnabledInput.setCurrentState(AUI::Checkbox::State::Unchecked);
    }
}

void AnimationPropertiesWindow::onAnimationRemoved(AnimationID animationID)
{
    if (animationID == activeAnimationID) {
        activeAnimationID = NULL_ANIMATION_ID;
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

void AnimationPropertiesWindow::onAnimationDisplayNameChanged(
    AnimationID animationID, const std::string& newDisplayName)
{
    if (animationID == activeAnimationID) {
        nameInput.setText(newDisplayName);
    }
}

void AnimationPropertiesWindow::onAnimationFrameCountChanged(
    AnimationID animationID, Uint8 newFrameCount)
{
    if (animationID == activeAnimationID) {
        frameCountInput.setText(std::to_string(newFrameCount));
    }
}

void AnimationPropertiesWindow::onAnimationFpsChanged(AnimationID animationID,
                                                      Uint8 newFps)
{
    if (animationID == activeAnimationID) {
        fpsInput.setText(std::to_string(newFps));
    }
}

void AnimationPropertiesWindow::onAnimationCollisionEnabledChanged(
    AnimationID animationID, bool newCollisionEnabled)
{
    if (animationID == activeAnimationID) {
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

void AnimationPropertiesWindow::onAnimationModelBoundsIDChanged(
    AnimationID animationID, BoundingBoxID newModelBoundsID)
{
    // If the animation isn't active, do nothing.
    if (animationID != activeAnimationID) {
        return;
    }

    // Whether they're enabled or not, the fields should show the correct bounds.
    const EditorAnimation& animation{
        dataModel.animationModel.getAnimation(animationID)};

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
        animation.getModelBounds(dataModel.boundingBoxModel)};
    minXInput.setText(toRoundedString(newModelBounds.min.x));
    minYInput.setText(toRoundedString(newModelBounds.min.y));
    minZInput.setText(toRoundedString(newModelBounds.min.z));
    maxXInput.setText(toRoundedString(newModelBounds.max.x));
    maxYInput.setText(toRoundedString(newModelBounds.max.y));
    maxZInput.setText(toRoundedString(newModelBounds.max.z));
}

void AnimationPropertiesWindow::onAnimationCustomModelBoundsChanged(
    AnimationID animationID, const BoundingBox& newCustomModelBounds)
{
    if (animationID == activeAnimationID) {
        minXInput.setText(toRoundedString(newCustomModelBounds.min.x));
        minYInput.setText(toRoundedString(newCustomModelBounds.min.y));
        minZInput.setText(toRoundedString(newCustomModelBounds.min.z));
        maxXInput.setText(toRoundedString(newCustomModelBounds.max.x));
        maxYInput.setText(toRoundedString(newCustomModelBounds.max.y));
        maxZInput.setText(toRoundedString(newCustomModelBounds.max.z));
    }
}
void AnimationPropertiesWindow::onLibrarySelectedItemsChanged(
    const std::vector<LibraryListItem*>& selectedItems)
{
    // If there's no active animation, do nothing.
    if (!activeAnimationID) {
        return;
    }

    // If a bounding box is selected, allow the user to assign it.
    if ((selectedItems.size() > 0)
        && (selectedItems[0]->type == LibraryListItem::Type::BoundingBox)) {
        boundingBoxButton.text.setText("Assign");
    }
    // If we have a shared bounding box assigned, allow the user to switch 
    // to a custom bounding box.
    else if (dataModel.animationModel.getAnimation(activeAnimationID)
                 .modelBoundsID
             != NULL_BOUNDING_BOX_ID) {
        boundingBoxButton.text.setText("Custom");
    }
    else {
        // Custom bounding box and no selection. Allow the user to save it.
        boundingBoxButton.text.setText("Save as");
    }
}

void AnimationPropertiesWindow::setBoundsFieldsEnabled(bool isEnabled)
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

std::string AnimationPropertiesWindow::toRoundedString(float value)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(3) << value;
    return stream.str();
}

void AnimationPropertiesWindow::saveName()
{
    dataModel.animationModel.setAnimationDisplayName(activeAnimationID,
                                               nameInput.getText());
}

void AnimationPropertiesWindow::saveFrameCount()
{
    // Validate the user input as a valid value.
    try {
        // Convert the input string to an int.
        int newFrameCount{std::stoi(frameCountInput.getText())};

        // Clamp the value to its bounds.
        // Note: There must be at least 1 frame in every animation.
        // Note: 1000 was chosen randomly. Adjust it if it's a problem.
        newFrameCount = std::clamp(newFrameCount, 1, 1000);

        // Apply the new value.
        dataModel.animationModel.setAnimationFrameCount(activeAnimationID,
                                                        newFrameCount);
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        frameCountInput.setText(std::to_string(committedFrameCount));
    }
}

void AnimationPropertiesWindow::saveFps()
{
    // Validate the user input as a valid value.
    try {
        // Convert the input string to an int.
        int newFps{std::stoi(fpsInput.getText())};

        // Clamp the value to its bounds.
        // Note: FPS must be at least 1.
        // Note: 1000 was chosen randomly. Adjust it if it's a problem.
        newFps = std::clamp(newFps, 1, 1000);

        // Apply the new value.
        dataModel.animationModel.setAnimationFps(activeAnimationID, newFps);
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        fpsInput.setText(std::to_string(committedFps));
    }
}

void AnimationPropertiesWindow::onBoundingBoxButtonPressed()
{
    AnimationModel& animationModel{dataModel.animationModel};

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
                animationModel.setAnimationModelBoundsID(
                    activeAnimationID,
                    static_cast<BoundingBoxID>(selectedItem->ID));

                break;
            }
        }
    }
    else if (buttonText == "Custom") {
        // If the animation isn't already using a custom bounding box, set it.
        if (animationModel.getAnimation(activeAnimationID).modelBoundsID
            != NULL_BOUNDING_BOX_ID) {
            animationModel.setAnimationModelBoundsID(activeAnimationID,
                                                     NULL_BOUNDING_BOX_ID);
        }
    }
    else if (buttonText == "Save as") {
        // If the animation is using a custom bounding box, open the "Save as" 
        // menu.
        const EditorAnimation& animation{
            animationModel.getAnimation(activeAnimationID)};
        if (animation.modelBoundsID == NULL_BOUNDING_BOX_ID) {
            mainScreen.openSaveBoundingBoxDialog(
                animation.customModelBounds,
                [&](BoundingBoxID newModelBoundsID) {
                    // The save was completed, set the shared bounding box as
                    // this animation's model bounds.
                    dataModel.animationModel.setAnimationModelBoundsID(
                        activeAnimationID, newModelBoundsID);
                });
        }
    }
}

void AnimationPropertiesWindow::saveMinX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinX{std::stof(minXInput.getText())};

        // Clamp the value to its bounds.
        const EditorAnimation& activeAnimation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        BoundingBox newModelBounds(
            activeAnimation.getModelBounds(dataModel.boundingBoxModel));
        newModelBounds.min.x = std::clamp(newMinX, 0.f, newModelBounds.max.x);

        // Apply the new value.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinX));
    }
}

void AnimationPropertiesWindow::saveMinY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinY{std::stof(minYInput.getText())};

        // Clamp the value to its bounds.
        const EditorAnimation& activeAnimation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        BoundingBox newModelBounds(
            activeAnimation.getModelBounds(dataModel.boundingBoxModel));
        newModelBounds.min.y = std::clamp(newMinY, 0.f, newModelBounds.max.y);

        // Apply the new value.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinY));
    }
}

void AnimationPropertiesWindow::saveMinZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMinZ{std::stof(minZInput.getText())};

        // Clamp the value to its bounds.
        const EditorAnimation& activeAnimation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        BoundingBox newModelBounds(
            activeAnimation.getModelBounds(dataModel.boundingBoxModel));
        newModelBounds.min.z = std::clamp(newMinZ, 0.f, newModelBounds.max.z);

        // Apply the new value.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        minXInput.setText(std::to_string(committedMinZ));
    }
}

void AnimationPropertiesWindow::saveMaxX()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxX{std::stof(maxXInput.getText())};

        // Clamp the value to its bounds.
        const EditorAnimation& activeAnimation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        AM_ASSERT(activeAnimation.frames.size() > 0,
                  "Animation must always have at least 1 frame.");
        const EditorSprite& firstSprite{activeAnimation.frames[0].sprite.get()};
        BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
            firstSprite.textureExtent, firstSprite.stageOrigin)};

        BoundingBox newModelBounds(
            activeAnimation.getModelBounds(dataModel.boundingBoxModel));
        newModelBounds.max.x
            = std::clamp(newMaxX, newModelBounds.min.x, stageWorldExtent.max.x);

        // Apply the new value.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxXInput.setText(std::to_string(committedMaxX));
    }
}

void AnimationPropertiesWindow::saveMaxY()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxY{std::stof(maxYInput.getText())};

        // Clamp the value to its bounds.
        const EditorAnimation& activeAnimation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        AM_ASSERT(activeAnimation.frames.size() > 0,
                  "Animation must always have at least 1 frame.");
        const EditorSprite& firstSprite{activeAnimation.frames[0].sprite.get()};
        BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
            firstSprite.textureExtent, firstSprite.stageOrigin)};

        BoundingBox newModelBounds(
            activeAnimation.getModelBounds(dataModel.boundingBoxModel));
        newModelBounds.max.y
            = std::clamp(newMaxY, newModelBounds.min.y,
                         static_cast<float>(SharedConfig::TILE_WORLD_WIDTH));

        // Apply the new value.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void AnimationPropertiesWindow::saveMaxZ()
{
    // Validate the user input as a valid float.
    try {
        // Convert the input string to a float.
        float newMaxZ{std::stof(maxZInput.getText())};

        // Clamp the value to its lower bound.
        // Note: We don't clamp to an upper bound cause it's hard to calc
        //       and not very useful. Can add if we ever care to.
        const EditorAnimation& activeAnimation{
            dataModel.animationModel.getAnimation(activeAnimationID)};
        AM_ASSERT(activeAnimation.frames.size() > 0,
                  "Animation must always have at least 1 frame.");
        const EditorSprite& firstSprite{activeAnimation.frames[0].sprite.get()};
        BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
            firstSprite.textureExtent, firstSprite.stageOrigin)};

        BoundingBox newModelBounds(
            activeAnimation.getModelBounds(dataModel.boundingBoxModel));
        newModelBounds.max.z
            = std::clamp(newMaxZ, newModelBounds.min.z, stageWorldExtent.max.z);

        // Apply the new value.
        dataModel.animationModel.setAnimationCustomModelBounds(
            activeAnimationID, BoundingBox(newModelBounds));
    } catch (std::exception&) {
        // Input was not valid, reset the field to what it was.
        maxYInput.setText(std::to_string(committedMaxY));
    }
}

void AnimationPropertiesWindow::saveCollisionEnabled()
{
    bool collisionEnabled{(collisionEnabledInput.getCurrentState()
                           == AUI::Checkbox::State::Checked)};
    dataModel.animationModel.setAnimationCollisionEnabled(activeAnimationID,
                                                    collisionEnabled);
}

} // End namespace ResourceImporter
} // End namespace AM
