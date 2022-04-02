#pragma once

#include "AUI/Screen.h"
#include "AUI/Thumbnail.h"
#include <string>

namespace AM
{
class AssetCache;

namespace Client
{
/**
 * The thumbnail style used for the main screen.
 */
class MainThumbnail : public AUI::Thumbnail
{
public:
    MainThumbnail(AssetCache& assetCache,
                  const std::string& inDebugName = "MainThumbnail");
};

} // End namespace Client
} // End namespace AM
