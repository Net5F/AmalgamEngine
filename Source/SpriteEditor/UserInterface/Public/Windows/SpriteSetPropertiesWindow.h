#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainTextInput.h"
#include "LibraryItemData.h"
#include "SpriteSets.h"

namespace AM
{
struct BoundingBox;

namespace SpriteEditor
{
class SpriteDataModel;

/**
 * The properties window shown when the user loads a sprite set from the Library.
 * Allows the user to edit the active sprite set's properties.
 */
class SpriteSetPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteSetPropertiesWindow(SpriteDataModel& inSpriteDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in an
        Editor<type>SpriteSet class. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

private:
    /**
     * If the new active item is a sprite, loads it's data into this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If type/ID matches active set) Sets this panel back to its default state.
     */
    void onSpriteSetRemoved(SpriteSet::Type type, Uint16 spriteSetID);

    /**
     * (If type/ID matches active set) Updates this panel with the active 
     * sprite set's new properties.
     */
    void onSpriteSetDisplayNameChanged(SpriteSet::Type type, Uint16 spriteSetID,
                               const std::string& newDisplayName);

    /**
     * Loads the given sprite set's data into this panel.
     */
    template<typename T>
    void loadActiveSpriteSet(SpriteSet::Type spriteSetType,
                             const T& newActiveSpriteSet);

    /** Used while setting user-inputted sprite set data. */
    SpriteDataModel& spriteDataModel;

    /** The active sprite set's type. */
    SpriteSet::Type activeSpriteSetType;

    /** The active sprite set's ID. */
    Uint16 activeSpriteSetID;

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;
};

} // End namespace SpriteEditor
} // End namespace AM
