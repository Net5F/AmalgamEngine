#pragma once

#include "LibraryItemData.h"
#include "LibraryListItem.h"
#include "GraphicSets.h"
#include "GraphicSetSlot.h"
#include "AUI/Window.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;
class LibraryWindow;

/**
 * The center stage shown when the user loads an Entity graphic set.
 *
 * Allows the user to edit the active graphic set's graphic slots.
 *
 * Note: This is separate from GraphicSetEditView because entity graphic sets 
 *       have significant differences from the others.
 */
class EntityGraphicSetEditView : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    EntityGraphicSetEditView(DataModel& inDataModel,
                       const LibraryWindow& inLibraryWindow);

private:
    /**
     * If the new active item is an entity graphic set, loads it's data into 
     * this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If ID matches active set) Sets this panel back to its default state.
     */
    void onEntityRemoved(EntityGraphicSetID graphicSetID);

    /**
     * (If ID matches active set) Sets the given graphic type to the given 
     * graphic, adding or removing slot widgets as necessary.
     */
    void onEntitySlotChanged(EntityGraphicSetID graphicSetID,
                             EntityGraphicType graphicType,
                             Rotation::Direction direction,
                             GraphicID newGraphicID);

    /**
     * Attempts to assign the currently selected library item to the 
     * given graphic type's slot.
     */
    void onAssignButtonPressed(EntityGraphicType graphicType,
                               Rotation::Direction direction);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /**
     * Fills graphicContainer with empty slot widgets for each EntityGraphicType.
     */
    void initGraphicContainer();

    /**
     * Fills the given slot widget with the given graphic's image and name.
     */
    void fillSlotGraphicData(GraphicSetSlot& slot, GraphicID graphicID);

    /**
     * Converts an entity graphic type and rotation into the associated 
     * graphicContainer index.
     */
    std::size_t toIndex(EntityGraphicType graphicType,
                        Rotation::Direction direction);

    /**
     * Iterates all of the engine and project entity graphic types, skipping the 
     * gaps.
     *
     * @param callback A callback of form void(EntityGraphicType).
     *
     * Note: If we ever need this elsewhere, we can move it to 
     *       EntityGraphicType.h.
     */
    template<typename Func>
    void iterateEntityGraphicTypes(Func callback);

    /** Used to get the current working dir when displaying the graphic. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    const LibraryWindow& libraryWindow;

    /** The active graphic set's ID. */
    EntityGraphicSetID activeGraphicSetID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;
    AUI::Text modifyText;
    AUI::Text clearText;

    /** Holds a slot for each EntityGraphicType and Rotation::Direction, in 
        order. */
    AUI::VerticalGridContainer graphicContainer;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
