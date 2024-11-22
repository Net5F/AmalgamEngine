#include "AnimationElementsWindow.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "AnimationID.h"
#include "Paths.h"
#include "SharedConfig.h"

namespace AM
{
namespace ResourceImporter
{
AnimationElementsWindow::AnimationElementsWindow(MainScreen& inScreen,
                                                 DataModel& inDataModel)
: AUI::Window({1617, 882, 303, 121}, "AnimationElementsWindow")
, mainScreen{inScreen}
, dataModel{inDataModel}
, activeAnimationID{NULL_ANIMATION_ID}
, boundingBoxListItem{{1, 40, (logicalExtent.w - 2), 40}, "Bounding Box"}
, entityAlignmentAnchorListItem{{1, 80, (logicalExtent.w - 2), 40},
                                "Entity Alignment Anchor"}
, backgroundImage{{0, 0, logicalExtent.w, logicalExtent.h},
                  "ElementsBackground"}
, headerImage{{0, 0, 303, 40}, "ElementsHeader"}
, windowLabel{{12, 0, 282, 40}, "ElementsWindowLabel"}
, addEntityAlignmentAnchorButton{{257, 89, 22, 22}, "AddAnchorButton"}
, remEntityAlignmentAnchorButton{{257, 89, 22, 22}, "RemAnchorButton"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(backgroundImage);
    children.push_back(headerImage);
    children.push_back(windowLabel);
    children.push_back(boundingBoxListItem);
    children.push_back(entityAlignmentAnchorListItem);
    children.push_back(addEntityAlignmentAnchorButton);
    children.push_back(remEntityAlignmentAnchorButton);

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
    styleLabel(windowLabel, "Animation Elements", 21);
    windowLabel.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);

    /* List items */
    boundingBoxListItem.setOnSelected(
        [this](AnimationElementsListItem*) {
            // Deselect the other list items and signal the new selection.
            entityAlignmentAnchorListItem.deselect();
            if (onListItemSelected) {
                onListItemSelected(ElementType::BoundingBox);
            }
        });
    entityAlignmentAnchorListItem.setOnSelected(
        [this](AnimationElementsListItem*) {
            // Deselect the other list items and signal the new selection.
            boundingBoxListItem.deselect();
            if (onListItemSelected) {
                onListItemSelected(ElementType::EntityAlignmentAnchor);
            }
        });

    /* Buttons */
    addEntityAlignmentAnchorButton.normalImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Icons/Plus.png");
    addEntityAlignmentAnchorButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Icons/PlusHovered.png");
    addEntityAlignmentAnchorButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Icons/Plus.png");
    addEntityAlignmentAnchorButton.text.setFont(
        (Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    addEntityAlignmentAnchorButton.text.setText("");
    addEntityAlignmentAnchorButton.setOnPressed([this]() {
        // Add a default anchor to the active animation.
        if (activeAnimationID) {
            dataModel.animationModel.setAnimationEntityAlignmentAnchor(
                activeAnimationID, Vector3{0, 0, 0});
        }
    });
    remEntityAlignmentAnchorButton.normalImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Icons/Minus.png");
    remEntityAlignmentAnchorButton.hoveredImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Icons/MinusHovered.png");
    remEntityAlignmentAnchorButton.pressedImage.setSimpleImage(
        Paths::TEXTURE_DIR + "Icons/Minus.png");
    remEntityAlignmentAnchorButton.text.setFont(
        (Paths::FONT_DIR + "B612-Regular.ttf"), 33);
    remEntityAlignmentAnchorButton.text.setText("");
    remEntityAlignmentAnchorButton.setOnPressed([this]() {
        // Remove the anchor from the active animation.
        if (activeAnimationID) {
            dataModel.animationModel.setAnimationEntityAlignmentAnchor(
                activeAnimationID, std::nullopt);
        }
    });

    // When the active animation is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&AnimationElementsWindow::onActiveLibraryItemChanged>(*this);
    AnimationModel& animationModel{dataModel.animationModel};
    animationModel.animationRemoved
        .connect<&AnimationElementsWindow::onAnimationRemoved>(*this);
    animationModel.animationEntityAlignmentAnchorChanged.connect<
        &AnimationElementsWindow::onAnimationEntityAlignmentAnchorChanged>(
        this);
}

void AnimationElementsWindow::setOnListItemSelected(
    std::function<void(ElementType)> inOnListItemSelected)
{
    onListItemSelected = inOnListItemSelected;
}

void AnimationElementsWindow::onActiveLibraryItemChanged(
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
    updateAnchorListItemState(newActiveAnimation->entityAlignmentAnchor);

    // Default to the bounding box being selected.
    boundingBoxListItem.select();
    entityAlignmentAnchorListItem.deselect();
}

void AnimationElementsWindow::onAnimationEntityAlignmentAnchorChanged(
    AnimationID animationID,
    const std::optional<Vector3>& newEntityAlignmentAnchor)
{
    updateAnchorListItemState(newEntityAlignmentAnchor);
}

void AnimationElementsWindow::onAnimationRemoved(AnimationID animationID)
{
    if (animationID == activeAnimationID) {
        activeAnimationID = NULL_ANIMATION_ID;
    }
}

void AnimationElementsWindow::updateAnchorListItemState(
    const std::optional<Vector3>& entityAlignmentAnchor)
{
    // If the anchor is non-null, show the '-' icon and enable the anchor list
    // item.
    if (entityAlignmentAnchor) {
        entityAlignmentAnchorListItem.enable();
        entityAlignmentAnchorListItem.select();
        remEntityAlignmentAnchorButton.setIsVisible(true);
        addEntityAlignmentAnchorButton.setIsVisible(false);
    }
    else {
        // Anchor is null. Disable the anchor list item, show the '+' icon, 
        // and select the default list item.
        entityAlignmentAnchorListItem.disable();
        boundingBoxListItem.select();
        addEntityAlignmentAnchorButton.setIsVisible(true);
        remEntityAlignmentAnchorButton.setIsVisible(false);
    }
}

} // End namespace ResourceImporter
} // End namespace AM
