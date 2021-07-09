#pragma once

#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainTextInput.h"

namespace AM
{
namespace SpriteEditor
{

class MainScreen;
class SpriteStaticData;

/**
 * The right-side panel on the main screen. Allows the user to view and
 * modify the active sprite's properties.
 */
class PropertiesPanel : public AUI::Component
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    PropertiesPanel(MainScreen& inScreen);

    /**
     * Loads the MainScreen's current active sprite's data into this panel.
     */
    void loadActiveSprite();

    /**
     * Clears all of the text inputs, putting this panel back in its default
     * state.
     */
    void clear();

    /** All fields below directly match a data field in SpriteStaticData.
        See its displayName and modelBounds fields for more information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

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

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to load and save sprite data. */
    MainScreen& mainScreen;

    AUI::Image backgroundImage;

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();
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
};

} // End namespace SpriteEditor
} // End namespace AM
