#include <Timer.h>
#include "SDL.h"

namespace AM
{
Timer::Timer()
: period(1.0 / SDL_GetPerformanceFrequency())
, savedTimestamp(0)
{
}

double Timer::getDeltaSeconds(bool updateSavedTime)
{
    Uint64 currentTicks = SDL_GetPerformanceCounter();
    Uint64 deltaTicks = currentTicks - savedTimestamp;

    if (updateSavedTime) {
        savedTimestamp = currentTicks;
    }

    return deltaTicks * period;
}

void Timer::updateSavedTime()
{
    savedTimestamp = SDL_GetPerformanceCounter();
}

} // namespace AM
