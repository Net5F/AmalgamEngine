#pragma once

#include <string>

namespace AM
{
/**
 * Used for accessing files in expected directories.
 *
 * The Client and SpriteEditor expect a particular asset directory
 * structure, starting at the binary's location:
 * .
 * └── Assets
 *     ├── Fonts
 *     └── Textures
 */
class Paths
{
public:
    /** The path that the application was ran from.
        Note: Use this instead of SDL_GetBasePath() to easily avoid leaking
              memory. */
    static const std::string BASE_PATH;

    /** The expected path to the root of the Fonts directory. */
    static const std::string FONT_DIR;

    /** The expected path to the root of the Textures directory. */
    static const std::string TEXTURE_DIR;
};

} // End namespace AM
