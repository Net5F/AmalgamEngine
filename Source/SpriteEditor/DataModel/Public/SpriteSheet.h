#pragma once

#include "SpriteStaticData.h"
#include <filesystem>
#include <vector>

namespace AM
{
namespace SpriteEditor
{

/**
 * Holds the path to a sprite sheet and all associated sprite data.
 */
struct SpriteSheet
{
public:
    std::filesystem::path path;

    /** The static data for all sprites in this sheet. */
    std::vector<SpriteStaticData> sprites;
};

} // namespace SpriteEditor
} // namespace AM
