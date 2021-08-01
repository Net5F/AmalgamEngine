#pragma once

#include "AUI/TextInput.h"
#include <string>

namespace AM
{
class AssetCache;

namespace SpriteEditor
{

/**
 * The text input style used for the main screen.
 */
class MainTextInput : public AUI::TextInput
{
public:
    MainTextInput(AssetCache& assetCache, AUI::Screen& inScreen, const SDL_Rect& inScreenExtent, const std::string& inDebugName = "");
};

} // End namespace SpriteEditor
} // End namespace AM
