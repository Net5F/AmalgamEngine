#ifndef TIMER_H
#define TIMER_H

#include "SDL_stdinc.h"

namespace AM
{

/**
 * Uses the SDL high resolution timer to produce time deltas.
 */
class Timer {
public:
    Timer();

    /**
     * Gets the time since the internally saved timestamp was last updated.
     * Note: The first time you call this function, it will return a large number
     *       (the time since SDL initialized the counter).
     *
     * @param updateSavedTime  If true, updates the internal timestamp.
     * @return The time delta in seconds since the saved time was last updated.
     */
    double getDeltaSeconds(bool updateSavedTime);

    /**
     * Updates the internal timestamp to the current time.
     */
    void updateSavedTime();

private:
    /**
     * How fast the processor is running.
     * SDL sets this once on init and never changes it.
     */
    const double period;

    // The saved time in integer ticks from SDL_GetPerformanceCounter().
    Uint64 savedTimestamp;
};

} // namespace AM

#endif /* TIMER_H */
