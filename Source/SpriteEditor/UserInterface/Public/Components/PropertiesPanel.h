#pragma once

#include "AUI/Image.h"

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
    PropertiesPanel(MainScreen& inScreen);

    /**
     * Loads the given sprite's data into this panel.
     */
    void loadSprite(const SpriteStaticData& sprite);

    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to load and save sprite data. */
    MainScreen& mainScreen;

    AUI::Image backgroundImage;
};

} // End namespace SpriteEditor
} // End namespace AM
