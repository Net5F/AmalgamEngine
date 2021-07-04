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
     * Loads the given sprite's data into this panel.
     */
    void loadSprite(const SpriteStaticData& sprite);

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
    /**
     * Clears the text in all of this dialog's text inputs.
     */
    void clearTextInputs();

    /** Used to load and save sprite data. */
    MainScreen& mainScreen;

    AUI::Image backgroundImage;
};

} // End namespace SpriteEditor
} // End namespace AM
