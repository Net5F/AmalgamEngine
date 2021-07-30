#pragma once

#include "AUI/Screen.h"
#include "AUI/Thumbnail.h"
#include <SDL_Rect.h>
#include <string>

namespace AM
{
namespace SpriteEditor
{

/**
 * The thumbnail style used for the main screen.
 */
class MainThumbnail : public AUI::Thumbnail
{
public:
    MainThumbnail(AUI::Screen& inScreen, const std::string& inDebugName = "");
};

} // End namespace SpriteEditor
} // End namespace AM
