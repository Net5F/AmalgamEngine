#pragma once

#include <SDL_stdinc.h>

namespace AM
{
/**
 * Uses the SDL high resolution timer to produce time deltas.
 */
class Timer
{
public:
    Timer();

    /**
     * Returns the amount of time that this timer has been running.
     *
     * @return The amount of time in seconds since the saved time was last
     *         updated.
     */
    double getTime();

    /**
     * Resets this timer to 0.
     */
    void reset();

    /**
     * Returns the amount of time that this timer has been running, then
     * resets it to 0.
     */
    double getTimeAndReset();

private:
    /** How fast the processor is running. SDL sets this once on init and 
        never changes it.  */
    double period;

    /** The saved time in integer ticks from SDL_GetPerformanceCounter(). */
    Uint64 savedTimestamp;
};

} // namespace AM
