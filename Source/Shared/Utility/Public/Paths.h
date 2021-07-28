#pragma once

#include <string>

namespace AM
{

/**
 * Module-specific paths. Used for accessing files in expected directories.
 */
class Paths
{
public:
    /** The path that the application was ran from.
        Note: Use this instead of SDL_GetBasePath() to easily avoid leaking
              memory. */
    static const char* BASE_PATH;

    /** The expected path to the root of the Textures directory. */
    static const std::string TEXTURE_DIR;

    /** The expected path to the root of the Fonts directory. */
    static const std::string FONT_DIR;
};

} // End namespace AM
