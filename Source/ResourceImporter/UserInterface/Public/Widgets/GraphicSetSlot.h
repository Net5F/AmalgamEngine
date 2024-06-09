#pragma once

#include "AUI/Widget.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainButton.h"

namespace AM
{
namespace ResourceImporter
{
/**
 * Allows the user to assign one of the graphics in a graphic set.
 */
class GraphicSetSlot : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    /**
     * @param logicalWidth The widget's width. Only affects the text width, the 
     *                     other elements are all centered.
     */
    GraphicSetSlot(int logicalWidth);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Top text, says what kind of graphic goes in this slot. */
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for this sprite. */
    AUI::Image checkerboardImage;

    /** The sprite that is in this slot. */
    AUI::Image spriteImage;

    /** The sprite's display name. */
    AUI::Text spriteNameText;

    /** The "ASSIGN" button, for assigning a new graphic to this slot. */
    MainButton assignButton;
};

} // End namespace ResourceImporter
} // End namespace AM
