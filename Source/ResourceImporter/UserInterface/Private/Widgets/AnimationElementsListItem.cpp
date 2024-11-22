#include "AnimationElementsListItem.h"
#include "Paths.h"
#include "AUI/Text.h"

namespace AM
{
namespace ResourceImporter
{
AnimationElementsListItem::AnimationElementsListItem(
    const SDL_Rect& inLogicalExtent, const std::string& inText,
    const std::string& inDebugName)
: AUI::Widget(inLogicalExtent, inDebugName)
, hoveredImage({0, 0, logicalExtent.w, logicalExtent.h})
, selectedImage({0, 0, logicalExtent.w, logicalExtent.h})
, text({0, 0, logicalExtent.w, logicalExtent.h})
, currentState{State::Normal}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(hoveredImage);
    children.push_back(selectedImage);
    children.push_back(text);

    // Add our backgrounds.
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "Highlights/Hovered.png");
    selectedImage.setSimpleImage(Paths::TEXTURE_DIR
                                 + "Highlights/Selected.png");

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    text.setText(inText);
    setLeftPadding(12);

    // Make the images we aren't using invisible.
    hoveredImage.setIsVisible(false);
    selectedImage.setIsVisible(false);
}

void AnimationElementsListItem::enable()
{
    SDL_Point cursorPosition{};
    SDL_GetMouseState(&(cursorPosition.x), &(cursorPosition.y));
    cursorPosition.x -= clippedExtent.x;
    cursorPosition.y -= clippedExtent.y;

    // Check if we're currently hovered.
    if (containsPoint(cursorPosition)) {
        setCurrentState(State::Hovered);
    }
    else {
        setCurrentState(State::Normal);
    }
}

void AnimationElementsListItem::disable()
{
    setCurrentState(State::Disabled);
}

void AnimationElementsListItem::select()
{
    // If we are already selected, do nothing.
    if (currentState == State::Selected) {
        return;
    }

    // Flag that we're now selected.
    setCurrentState(State::Selected);

    // If the user set a callback for this event, call it.
    if (onSelected) {
        onSelected(this);
    }
}

void AnimationElementsListItem::deselect()
{
    // If we aren't selected, do nothing.
    if (currentState != State::Selected) {
        return;
    }

    // A different list item must have been selected, so we know that we 
    // aren't hovered. Go to normal.
    setCurrentState(State::Normal);

    // If the user set a callback for this event, call it.
    if (onDeselected) {
        onDeselected(this);
    }
}

AnimationElementsListItem::State
    AnimationElementsListItem::getCurrentState() const
{
    return currentState;
}

void AnimationElementsListItem::setLeftPadding(int inLeftPadding)
{
    text.setLogicalExtent(
        {inLeftPadding, 0, (logicalExtent.w - inLeftPadding), logicalExtent.h});
}

void AnimationElementsListItem::setOnSelected(
    std::function<void(AnimationElementsListItem*)> inOnSelected)
{
    onSelected = std::move(inOnSelected);
}

void AnimationElementsListItem::setOnDeselected(
    std::function<void(AnimationElementsListItem*)> inOnDeselected)
{
    onDeselected = std::move(inOnDeselected);
}

AUI::EventResult AnimationElementsListItem::onMouseDown(AUI::MouseButtonType buttonType,
                                              const SDL_Point&)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }
    // If we're disabled, ignore the event.
    else if (currentState == State::Disabled) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we aren't already selected, select this widget.
    if (currentState != State::Selected) {
        select();
    }

    return AUI::EventResult{.wasHandled{true}};
}

AUI::EventResult AnimationElementsListItem::onMouseDoubleClick(
    AUI::MouseButtonType buttonType, const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

void AnimationElementsListItem::onMouseEnter()
{
    // If we're selected or disabled, ignore the event.
    if ((currentState == State::Selected)
        || (currentState == State::Disabled)) {
        return;
    }

    // If we're normal, change to hovered.
    if (currentState == State::Normal) {
        setCurrentState(State::Hovered);
    }
}

void AnimationElementsListItem::onMouseLeave()
{
    // If we're disabled, ignore the event.
    if (currentState == State::Disabled) {
        return;
    }

    // If we're hovered, go to normal.
    if (currentState == State::Hovered) {
        setCurrentState(State::Normal);
    }
}

void AnimationElementsListItem::setCurrentState(State inState)
{
    // Set the new state.
    currentState = inState;

    // Set all of our graphics to the default state.
    hoveredImage.setIsVisible(false);
    selectedImage.setIsVisible(false);
    text.setColor({255, 255, 255, 255});

    switch (currentState) {
        case State::Normal: {
            // Use defaults.
            break;
        }
        case State::Hovered: {
            hoveredImage.setIsVisible(true);
            break;
        }
        case State::Selected: {
            selectedImage.setIsVisible(true);
            break;
        }
        case State::Disabled: {
            text.setColor({166, 166, 166, 255});
            break;
        }
    }
}

} // End namespace ResourceImporter
} // End namespace AM
