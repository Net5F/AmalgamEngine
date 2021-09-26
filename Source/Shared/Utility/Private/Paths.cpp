#include "Paths.h"
#include <SDL2/SDL_filesystem.h>

namespace AM
{
// Note: It would be nice if we could trivially free this after making it a
//       string, but it isn't a big deal.
const char* BASE_PATH_INTERNAL = SDL_GetBasePath();

const std::string Paths::BASE_PATH = std::string{BASE_PATH_INTERNAL};

const std::string Paths::FONT_DIR
    = std::string{Paths::BASE_PATH} + "Assets/Fonts/";

const std::string Paths::TEXTURE_DIR
    = std::string{Paths::BASE_PATH} + "Assets/Textures/";

} // End namespace AM
