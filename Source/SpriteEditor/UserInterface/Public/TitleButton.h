#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"

namespace AM
{

/**
 * The button style used for the title screen.
 */
class TitleButton : public AUI::Button
{
public:
    TitleButton(AUI::Screen& screen, const char* key, const SDL_Rect& screenExtent
                , const std::string& inText);
};

} // End namespace AM
