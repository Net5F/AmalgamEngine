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
    TitleButton(AssetCache& assetCache, AUI::Screen& inScreen,
                const SDL_Rect& inScreenExtent, const std::string& inText,
                const std::string& inDebugName = "");
};

} // End namespace SpriteEditor
} // End namespace AM
