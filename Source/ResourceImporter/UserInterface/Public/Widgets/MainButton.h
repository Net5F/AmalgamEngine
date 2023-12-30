#pragma once

#include "AUI/Screen.h"
#include "AUI/Button.h"
#include <string>

namespace AM
{
namespace ResourceImporter
{
/**
 * The usual button style used for the main screen.
 */
class MainButton : public AUI::Button
{
public:
    MainButton(const SDL_Rect& inLogicalExtent, const std::string& inText,
               const std::string& inDebugName = "MainButton");
};

} // End namespace ResourceImporter
} // End namespace AM
