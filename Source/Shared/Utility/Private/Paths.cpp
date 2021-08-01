#include "Paths.h"
#include <SDL2/SDL_filesystem.h>

namespace AM
{

const char* Paths::BASE_PATH = SDL_GetBasePath();

const std::string Paths::FONT_DIR = std::string{Paths::BASE_PATH} + "Assets/Fonts/";

const std::string Paths::TEXTURE_DIR = std::string{Paths::BASE_PATH} + "Assets/Textures/";

} // End namespace AM
