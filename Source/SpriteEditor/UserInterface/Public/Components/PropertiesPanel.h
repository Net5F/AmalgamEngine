#pragma once

#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainTextInput.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{

class MainScreen;
class SpriteDataModel;
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
    PropertiesPanel(AssetCache& assetCache, MainScreen& inScreen, SpriteDataModel& inSpriteDataModel);

    /**
     * Loads the given sprite's data into this panel.
     */
    void loadActiveSprite(SpriteStaticData* inActiveSprite);

    /**
     * Refreshes this component's UI with the data from the currently set
     * active sprite.
     * Errors if activeSprite is nullptr.
     */
    void refresh();

    /**
     * Sets activeSprite to nullptr and clears all of the text inputs, putting
     * this panel back to its default state.
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
    /** Used to save updated sprite data. */
    MainScreen& mainScreen;

    /** Used while setting user-inputted sprite data. */
    SpriteDataModel& spriteDataModel;

    /** The active sprite's data. */
    SpriteStaticData* activeSprite;

    AUI::Image backgroundImage;

    /**
     * Converts the given float to a string with 3 decimals of precision.
     */
    std::string toRoundedString(float value);

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
