#pragma once

#include <SDL_render.h>
#include <string>

namespace AM
{
namespace ResourceImporter
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
    /** The actual size of the window we're rendering to. */
    static constexpr unsigned int ACTUAL_SCREEN_WIDTH{1600};
    static constexpr unsigned int ACTUAL_SCREEN_HEIGHT{900};

    /**
     * The logical screen size that we're targeting.
     *
     * The developer can write layout logic in reference to this logical size,
     * and it will be scaled to match the actual size above.
     */
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
     * Sets the quality of scaling/filtering algorithm used for sprite 
     * textures (i.e any textures rendered by Renderer).
     * 
     * UI textures ignore this setting.
     */
    static constexpr SDL_ScaleMode SPRITE_SCALING_QUALITY{SDL_ScaleModeLinear};

    /** The rate at which the user interfaces fires off tick() events. */
    static constexpr unsigned int UI_TICKS_PER_SECOND{30};
    static constexpr double UI_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(UI_TICKS_PER_SECOND)};
};

} // End namespace ResourceImporter
} // End namespace AM
