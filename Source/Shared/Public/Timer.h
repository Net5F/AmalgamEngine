#ifndef TIMER_H
#define TIMER_H

#include "SDL.h"

namespace AM
{

/**
 * Uses the SDL high resolution timer to produce time deltas.
 */
class Timer {
public:
    Timer();

    /**
     * Note: The first time you call this function, it will return a large number
     * (the time since SDL initialized the counter).
     *
     * @return The time delta since this function was last called in seconds.
     */
    float getDeltaSeconds();

private:
    /**
     * How fast the processor is running.
     * SDL sets this once on init and never changes it.
     */
    const Uint64 TICKS_PER_SECOND;

    Uint64 previousTicks;
};

} // namespace AM

#endif /* TIMER_H */
