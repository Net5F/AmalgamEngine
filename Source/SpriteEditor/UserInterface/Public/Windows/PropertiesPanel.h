#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Checkbox.h"
#include "MainTextInput.h"

namespace AM
{
struct BoundingBox;

namespace SpriteEditor
{
class SpriteDataModel;
struct Sprite;

/**
 * The right-side panel on the main screen. Allows the user to view and
 * modify the active sprite's properties.
 */
class PropertiesPanel : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    PropertiesPanel(SpriteDataModel& inSpriteDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the Sprite class.
        See displayName, collisionEnabled, and modelBounds fields for more
        information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

    AUI::Text collisionEnabledLabel;
    AUI::Checkbox collisionEnabledInput;

    AUI::Text minXLabel;
    MainTextInput minXInput;

    AUI::Text minYLabel;
    MainTextInput minYInput;

    AUI::Text minZLabel;
    MainTextInput minZInput;

    AUI::Text maxXLabel;
    MainTextInput maxXInput;

    AUI::Text maxYLabel;
    MainTextInput maxYInput;

    AUI::Text maxZLabel;
    MainTextInput maxZInput;

private:
    /**
     * Loads the new active sprite's data into this panel.
     */
    void onActiveSpriteChanged(unsigned int newSpriteID,
                               const Sprite& newActiveSprite);

    /**
     * (If active sprite was removed) Sets activeSprite to invalid and clears
     * all of the text inputs, putting this panel back to its default state.
     */
    void onSpriteRemoved(unsigned int spriteID);

    /**
     * (If active sprite changed) Updates this panel with the active sprite's
     * new properties.
     */
    void onSpriteDisplayNameChanged(unsigned int spriteID,
                                    const std::string& newDisplayName);
    void onSpriteCollisionEnabledChanged(unsigned int spriteID,
                                       bool newCollisionEnabled);
    void onSpriteModelBoundsChanged(unsigned int spriteID,
                                    const BoundingBox& newModelBounds);

    /** Used while setting user-inputted sprite data. */
    SpriteDataModel& spriteDataModel;

    /** The active sprite's ID. */
    unsigned int activeSpriteID;

    /**
     * Converts the given float to a string with 3 decimals of precision.
     */
    std::string toRoundedString(float value);

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();
    void saveCollisionEnabled();
    void saveMinX();
    void saveMinY();
    void saveMinZ();
    void saveMaxX();
    void saveMaxY();
    void saveMaxZ();

    /** The below floats save the committed values, so we can revert to them
        if the user inputs invalid characters. */
    float committedMinX;
    float committedMinY;
    float committedMinZ;
    float committedMaxX;
    float committedMaxY;
    float committedMaxZ;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;
};

} // End namespace SpriteEditor
} // End namespace AM
