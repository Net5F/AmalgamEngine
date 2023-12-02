#pragma once

#include "AUI/Button.h"

namespace AM
{
namespace ResourceImporter
{
/**
 * The button style used for the title screen.
 */
class TitleButton : public AUI::Button
{
public:
    TitleButton(const SDL_Rect& inLogicalExtent, const std::string& inText,
                const std::string& inDebugName = "TitleButton");
};

} // End namespace ResourceImporter
} // End namespace AM
