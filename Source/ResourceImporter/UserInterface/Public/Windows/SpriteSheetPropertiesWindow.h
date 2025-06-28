#pragma once

#include "SpriteSheetID.h"
#include "MainTextInput.h"
#include "MainButton.h"
#include "LibraryItemData.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"

namespace AM
{
struct BoundingBox;

namespace ResourceImporter
{
class MainScreen;
class DataModel;
class LibraryWindow;

/**
 * The properties window shown when the user loads a sprite from the Library.
 * Allows the user to edit the active sprite's properties.
 */
class SpriteSheetPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteSheetPropertiesWindow(MainScreen& inScreen, DataModel& inDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the EditorSprite class.
        See displayName, collisionEnabled, and modelBounds fields for more
        information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

    MainButton addImagesButton;

private:
    /**
     * If the new active item is a sprite, loads it's data into this window.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active sheet was removed) Hides this window.
     */
    void onSheetRemoved(SpriteSheetID parentSheetID);

    /**
     * Opens the sprite image file selector dialog.
     */
    void onAddImagesButtonPressed();

    /** Used to open the confirmation dialog when saving a bounding box. */
    MainScreen& mainScreen;

    /** Used while setting user-inputted sprite data. */
    DataModel& dataModel;

    /** The active sprite sheet's ID. */
    SpriteSheetID activeSpriteSheetID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;
};

} // End namespace ResourceImporter
} // End namespace AM
