#ifndef TIMER_H
#define TIMER_H

#include <SDL_stdinc.h>
#include <SDL_timer.h>

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
    Uint64 getDeltaCount(bool updateSavedTime);

    /**
     * Updates the internal timestamp to the current time.
     */
    void updateSavedCount();

    /**
     * Converts a high resolution timer count into an equivalent time in seconds.
     */
    static double countToSeconds(Uint64 count);

    /**
     * Converts a time in seconds to an equivalent high resolution timer count.
     */
    static Uint64 secondsToCount(double seconds);

    /**
     * Converts a desired number of iterations per second to an equivalent high resolution
     * timer count.
     */
    static Uint64 ipsToCount(unsigned int iterationsPerSecond);

private:
    /**
     * The high resolution counter value equivalent to 1 second.
     * Note: This variable is a singleton because it's used at static initialization time
     *       and would otherwise potentially be used before it was initialized.
     */
    static const Uint64& COUNT_PER_SECOND();

    // The saved high resolution timer count from SDL_GetPerformanceCounter().
    Uint64 savedCount;
};

} // namespace AM

#endif /* TIMER_H */
