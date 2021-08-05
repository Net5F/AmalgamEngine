#pragma once

#include <string>

namespace AM
{
namespace SpriteEditor
{
/**
 * Module-specific configuration data.
 *
 * All data is currently static, but eventually this class will be in charge
 * of loading some of the data dynamically from a config file.
 */
class Config
{
public:
    //    static constexpr unsigned int ACTUAL_SCREEN_WIDTH{1920};
    //    static constexpr unsigned int ACTUAL_SCREEN_HEIGHT{1080};
    static constexpr unsigned int ACTUAL_SCREEN_WIDTH{1600};
    static constexpr unsigned int ACTUAL_SCREEN_HEIGHT{900};
    //    static constexpr unsigned int ACTUAL_SCREEN_WIDTH{1280};
    //    static constexpr unsigned int ACTUAL_SCREEN_HEIGHT{720};

    static constexpr unsigned int LOGICAL_SCREEN_WIDTH{1920};
    static constexpr unsigned int LOGICAL_SCREEN_HEIGHT{1080};

    /**
     * Sets full screen preference.
     * 0 = windowed.
     * 1 = Real fullscreen.
     * 2 = Fullscreen windowed.
     */
    static constexpr unsigned int FULLSCREEN_MODE{0};

    /**
     * Sets the quality of scaling algorithm used.
     * "nearest" = Nearest pixel sampling.
     * "linear" = Linear filtering (supported by OpenGL and Direct3D).
     * "best" = Ansiotropic filtering (supported by Direct3D).
     */
    static constexpr char SCALING_QUALITY[]{"linear"};

    /** The rate at which the user interfaces fires off tick() events. */
    static constexpr unsigned int UI_TICKS_PER_SECOND = 30;
    static constexpr double UI_TICK_TIMESTEP_S
        = 1.0 / static_cast<double>(UI_TICKS_PER_SECOND);
};

} // End namespace SpriteEditor
} // End namespace AM
