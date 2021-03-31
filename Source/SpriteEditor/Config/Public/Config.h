#pragma once

namespace AM
{
namespace SpriteEditor
{

/**
 * This class contains module-specific configuration data.
 *
 * All data is currently static, but eventually this class will be in charge
 * of loading some of the data dynamically from a config file.
 */
class Config
{
public:
    static constexpr unsigned int SCREEN_WIDTH = 1280;
    static constexpr unsigned int SCREEN_HEIGHT = 720;

    static constexpr unsigned int LOGICAL_SCREEN_WIDTH = 1280;
    static constexpr unsigned int LOGICAL_SCREEN_HEIGHT = 720;

    /** Corresponds to SDL_HINT_RENDER_SCALE_QUALITY. */
    static constexpr char SCALING_QUALITY[] = "nearest";
};

} // End namespace SpriteEditor
} // End namespace AM
