#pragma once

#include "LibraryItemData.h"
#include "BoundingBoxGizmo.h"
#include "MainButton.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "AUI/Image.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;
class LibraryWindow;
class LibraryListItem;

/**
 * The center stage shown when the user loads a bounding box from the Library.
 * Allows the user to edit the bounding box, for use by sprites and animations.
 */
class BoundingBoxEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BoundingBoxEditStage(DataModel& inDataModel,
                         LibraryWindow& inLibraryWindow);

private:
    /**
     * If a sprite is selected in the library, adds it to the stage.
     */
    void onPreviewSpriteButtonPressed();

    /**
     * If the new active item is a bounding box, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * If the changed box is currently active, updates this stage to reflect 
     * the new data.
     */
    void onBoundingBoxBoundsChanged(BoundingBoxID boundingBoxID,
                                    const BoundingBox& newBounds);

    /**
     * (If active box was removed) Sets activeBoundingBoxID to invalid and 
     * returns the stage to its default state.
     */
    void onBoundingBoxRemoved(BoundingBoxID boundingBoxID);

    /**
     * Pushes the gizmo's updated bounding box to the model.
     */
    void onGizmoBoundingBoxUpdated(const BoundingBox& updatedBounds);

    /**
     * Updates previewSpriteButton to show whether the selection is preview-able.
     */
    void onLibrarySelectedItemsChanged(
        const std::vector<LibraryListItem*>& selectedItems);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /** Used to get the current working dir when displaying graphics. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    LibraryWindow& libraryWindow;

    /** The active bounding box's ID. */
    BoundingBoxID activeBoundingBoxID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;
    AUI::Text modifyText;

    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::Image checkerboardImage;

    /** The preview image that is currently loaded onto the stage, if any. */
    AUI::Image spriteImage;

    /** The gizmo for editing the sprite's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;

    /** Used to preview a sprite, to make drawing the bounding box easier. */
    MainButton previewSpriteButton;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
