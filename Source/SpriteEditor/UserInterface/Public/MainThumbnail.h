#pragma once

#include "AUI/Screen.h"
#include "AUI/Thumbnail.h"
#include <SDL_Rect.h>
#include <string>

namespace AM
{

/**
 * The thumbnail style used for the main screen.
 */
class MainThumbnail : public AUI::Thumbnail
{
public:
    MainThumbnail(AUI::Screen& screen, const char* key, const SDL_Rect& screenExtent);
};

} // End namespace AM
