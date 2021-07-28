#include "Paths.h"
#include <SDL_filesystem.h>

namespace AM
{
namespace SpriteEditor
{

const char* Paths::BASE_PATH = SDL_GetBasePath();

const std::string Paths::TEXTURE_DIR = std::string{Paths::BASE_PATH} + "Resources/Textures/";

const std::string Paths::FONT_DIR = std::string{Paths::BASE_PATH} + "Resources/Fonts/";

} // End namespace SpriteEditor
} // End namespace AM
