#pragma once

#include <string>

namespace AM
{
namespace SpriteEditor
{

/**
 * Module-specific paths. Used for accessing files in expected directories.
 */
class Paths
{
public:
    static const char* BASE_PATH;

    /** The expected path to the root of the Textures directory. */
    static const std::string TEXTURE_DIR;

    /** The expected path to the root of the Fonts directory. */
    static const std::string FONT_DIR;
};

} // End namespace SpriteEditor
} // End namespace AM
