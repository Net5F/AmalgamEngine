#pragma once

#include "AUI/Screen.h"
#include "AUI/Thumbnail.h"
#include <string>

namespace AM
{
namespace ResourceImporter
{
/**
 * The thumbnail style used for the main screen.
 */
class MainThumbnail : public AUI::Thumbnail
{
public:
    MainThumbnail(const std::string& inDebugName = "MainThumbnail");
};

} // End namespace ResourceImporter
} // End namespace AM
