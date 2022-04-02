#pragma once

#include "AUI/Button.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
/**
 * The button style used for the title screen.
 */
class TitleButton : public AUI::Button
{
public:
    TitleButton(AssetCache& assetCache,
                const SDL_Rect& inScreenExtent, const std::string& inText,
                const std::string& inDebugName = "TitleButton");
};

} // End namespace SpriteEditor
} // End namespace AM
