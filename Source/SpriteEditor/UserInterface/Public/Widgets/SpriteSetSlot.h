#pragma once

#include "AUI/Widget.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainButton.h"

namespace AM
{
namespace SpriteEditor
{
/**
 * Allows the user to assign one of the sprites in a sprite set.
 */
class SpriteSetSlot : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteSetSlot(const std::string& inText,
                  const std::string& inDebugName = "SpriteSetSlot");

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Top text, says what kind of sprite goes in this slot. */
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for this sprite. */
    AUI::Image checkerboardImage;

    /** The sprite that is in this slot. */
    AUI::Image spriteImage;

    /** The sprite's display name. */
    AUI::Text spriteNameText;

    /** The "ASSIGN" button, for assigning a new sprite to this slot. */
    MainButton assignButton;
};

} // End namespace SpriteEditor
} // End namespace AM
