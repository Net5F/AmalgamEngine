#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"
#include <string>

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
    ConfirmationButton(const SDL_Rect& inScreenExtent,
                       const std::string& inText,
                       const std::string& inDebugName = "ConfirmationButton");
};

} // End namespace SpriteEditor
} // End namespace AM
