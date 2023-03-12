#pragma once

#include "SpriteSheet.h"
#include "Sprite.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
namespace SpriteEditor
{
class SpriteDataModel;
class MainThumbnail;

/**
 * The bottom panel on the main screen. Allows the user to select sprites to
 * load onto the stage.
 */
class SpritePanel : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpritePanel(SpriteDataModel& inSpriteDataModel);

private:
    /**
     * Adds a new MainThumbnail for the given sprite to the spriteContainer.
     */
    void onSpriteAdded(unsigned int spriteID, const Sprite& sprite);

    /**
     * Removes the thumbnail associated with the given sprite.
     */
    void onSpriteRemoved(unsigned int sheetID);

    /**
     * Updates the display name on the thumbnail of the given sprite.
     */
    void onSpriteDisplayNameChanged(unsigned int spriteID,
                                    const std::string& newDisplayName);

    /** Used to get the current working dir when displaying the thumbnails. */
    SpriteDataModel& spriteDataModel;

    /** Maps sprite IDs to their associated thumbnails. */
    std::unordered_map<unsigned int, MainThumbnail*> thumbnailMap;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteContainer;
};

} // End namespace SpriteEditor
} // End namespace AM
