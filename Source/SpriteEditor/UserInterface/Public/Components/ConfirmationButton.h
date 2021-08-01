#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"
#include <string>

namespace AM
{
class AssetCache;

namespace SpriteEditor
{

/**
 * The button style used for the confirm/cancel buttons in confirmation
 * dialogs.
 */
class ConfirmationButton : public AUI::Button
{
public:
    ConfirmationButton(AssetCache& assetCache, AUI::Screen& inScreen, const SDL_Rect& inScreenExtent
                , const std::string& inText, const std::string& inDebugName = "");
};

} // End namespace SpriteEditor
} // End namespace AM
