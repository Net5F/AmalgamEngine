#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"

namespace AM
{
namespace SpriteEditor
{

/**
 * The button style used for the confirm/cancel buttons in confirmation
 * dialogs.
 */
class ConfirmationButton : public AUI::Button
{
public:
    ConfirmationButton(AUI::Screen& screen, const char* key, const SDL_Rect& screenExtent
                , const std::string& inText);
};

} // End namespace SpriteEditor
} // End namespace AM
