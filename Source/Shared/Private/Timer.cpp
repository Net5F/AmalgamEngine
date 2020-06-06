#include <Timer.h>
#include "SDL.h"

namespace AM
{

Timer::Timer()
: TICKS_PER_SECOND(SDL_GetPerformanceFrequency())
, savedTimestamp(0)
{
}

float Timer::getDeltaSeconds(bool updateSavedTime)
{
    Uint64 currentTicks = SDL_GetPerformanceCounter();
    Uint64 deltaTicks =  currentTicks - savedTimestamp;

    if (updateSavedTime) {
        savedTimestamp = currentTicks;
    }

    return deltaTicks / static_cast<float>(TICKS_PER_SECOND);
}

} // namespace AM
