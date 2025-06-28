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
class MainScreen;
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
    SpritePropertiesWindow(MainScreen& inScreen, DataModel& ineDataModel,
                           LibraryWindow& inLibraryWindow);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the EditorSprite class.
        See displayName, collisionEnabled, and modelBounds fields for more
        information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

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

    AUI::Text collisionEnabledLabel;
    AUI::Checkbox collisionEnabledInput;

    AUI::Text stageOriginXLabel;
    MainTextInput stageOriginXInput;

    AUI::Text stageOriginYLabel;
    MainTextInput stageOriginYInput;

    AUI::Text premultiplyAlphaLabel;
    AUI::Checkbox premultiplyAlphaInput;

private:
    /**
     * If the new active item is a sprite, loads it's data into this window.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active sprite was removed) Hides this window.
     */
    void onSpriteRemoved(SpriteID spriteID);

    /**
     * (If ID matches active sprite) Updates this panel with the active sprite's
     * new properties.
     */
    void onSpriteDisplayNameChanged(SpriteID spriteID,
                                    const std::string& newDisplayName);
    void onSpriteModelBoundsIDChanged(SpriteID spriteID,
                                      BoundingBoxID newModelBoundsID);
    void onSpriteCustomModelBoundsChanged(
        SpriteID spriteID, const BoundingBox& newCustomModelBounds);
    void onSpriteCollisionEnabledChanged(SpriteID spriteID,
                                         bool newCollisionEnabled);
    void onSpriteStageOriginChanged(SpriteID spriteID,
                                    const SDL_Point& newStageOrigin);
    void onSpritePremultiplyAlphaChanged(SpriteID spriteID,
                                         bool newPremultiplyAlpha);

    /**
     * Updates boundingBoxButton to show whether the selection is assignable.
     */
    void onLibrarySelectedItemsChanged(
        const std::vector<LibraryListItem*>& selectedItems);

    /**
     * Enables or disables the min/max bounds fields.
     */
    void setBoundsFieldsEnabled(bool isEnabled);

    /** Used to open the confirmation dialog when saving a bounding box. */
    MainScreen& mainScreen;

    /** Used while setting user-inputted sprite data. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    LibraryWindow& libraryWindow;

    /** The active sprite's ID. */
    SpriteID activeSpriteID;

    /**
     * Converts the given float to a string with 3 decimals of precision.
     */
    std::string toRoundedString(float value);

    /** Handles the context-sensitive bounding box button behavior. */
    void onBoundingBoxButtonPressed();

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveMinX();
    void saveMinY();
    void saveMinZ();
    void saveMaxX();
    void saveMaxY();
    void saveMaxZ();
    void saveCollisionEnabled();
    void saveStageOriginX();
    void saveStageOriginY();
    void savePremultiplyAlpha();

    /** The below variables save the committed values, so we can revert to them
        if the user inputs invalid characters. */
    float committedMinX;
    float committedMinY;
    float committedMinZ;
    float committedMaxX;
    float committedMaxY;
    float committedMaxZ;
    int committedStageOriginX;
    int committedStageOriginY;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;
};

} // End namespace ResourceImporter
} // End namespace AM
