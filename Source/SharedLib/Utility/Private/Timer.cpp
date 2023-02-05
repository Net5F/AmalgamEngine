#include "Timer.h"
#include "SDL.h"

namespace AM
{
Timer::Timer()
: period{1.0 / SDL_GetPerformanceFrequency()}
, savedTimestamp{SDL_GetPerformanceCounter()}
{
}

double Timer::getTime()
{
    Uint64 currentTicks{SDL_GetPerformanceCounter()};
    Uint64 deltaTicks{currentTicks - savedTimestamp};

    return deltaTicks * period;
}

void Timer::reset()
{
    savedTimestamp = SDL_GetPerformanceCounter();
}

double Timer::getTimeAndReset()
{
    Uint64 currentTicks{SDL_GetPerformanceCounter()};
    Uint64 deltaTicks{currentTicks - savedTimestamp};

    savedTimestamp = currentTicks;

    return deltaTicks * period;
}

} // namespace AM
