#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Checkbox.h"
#include "MainTextInput.h"
#include "MainButton.h"
#include "LibraryItemData.h"

namespace AM
{
struct BoundingBox;

namespace ResourceImporter
{
class DataModel;
class LibraryWindow;
class LibraryListItem;

/**
 * The properties window shown when the user loads a sprite from the Library.
 * Allows the user to edit the active sprite's properties.
 */
class SpritePropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpritePropertiesWindow(DataModel& ineDataModel,
                           LibraryWindow& inLibraryWindow);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the EditorSprite class.
        See displayName, collisionEnabled, and modelBounds fields for more
        information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

    AUI::Text collisionEnabledLabel;
    AUI::Checkbox collisionEnabledInput;

    AUI::Text boundingBoxLabel;
    AUI::Text boundingBoxNameLabel;
    MainButton boundingBoxButton;

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
     * If the new active item is a sprite, loads it's data into this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active sprite was removed) Sets this panel back to its default state.
     */
    void onSpriteRemoved(int spriteID);

    /**
     * (If ID matches active sprite) Updates this panel with the active sprite's
     * new properties.
     */
    void onSpriteDisplayNameChanged(int spriteID,
                                    const std::string& newDisplayName);
    void onSpriteCollisionEnabledChanged(int spriteID,
                                         bool newCollisionEnabled);
    void onSpriteModelBoundsIDChanged(int spriteID,
                                      BoundingBoxID newModelBoundsID);
    void onSpriteCustomModelBoundsChanged(
        int spriteID, const BoundingBox& newCustomModelBounds);

    /**
     * Updates boundingBoxButton to show whether the list item is assignable.
     */
    void onLibraryListItemSelected(const LibraryListItem& selectedItem);
    void onLibraryListItemDeselected(const LibraryListItem& deselectedItem);

    /**
     * Enables or disables the min/max bounds fields.
     */
    void setBoundsFieldsEnabled(bool isEnabled);

    /** Used while setting user-inputted sprite data. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    LibraryWindow& libraryWindow;

    /** The active sprite's ID. */
    int activeSpriteID;

    /**
     * Converts the given float to a string with 3 decimals of precision.
     */
    std::string toRoundedString(float value);

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();
    void saveCollisionEnabled();
    void saveModelBoundsID();
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

} // End namespace ResourceImporter
} // End namespace AM
