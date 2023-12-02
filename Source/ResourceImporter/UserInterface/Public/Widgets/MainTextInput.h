#pragma once

#include "AUI/TextInput.h"
#include <string>

namespace AM
{
namespace ResourceImporter
{
/**
 * The text input style used for the main screen.
 */
class MainTextInput : public AUI::TextInput
{
public:
    MainTextInput(const SDL_Rect& inLogicalExtent,
                  const std::string& inDebugName = "MainTextInput");
};

} // End namespace ResourceImporter
} // End namespace AM
