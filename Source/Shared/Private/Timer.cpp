#include <Timer.h>

namespace AM
{

Timer::Timer()
: TICKS_PER_SECOND(SDL_GetPerformanceFrequency())
, previousTicks(0)
{
}

float Timer::getDeltaSeconds()
{
    Uint64 currentTicks = SDL_GetPerformanceCounter();
    Uint64 deltaTicks =  currentTicks - previousTicks;
    previousTicks = currentTicks;

    return deltaTicks / static_cast<float>(TICKS_PER_SECOND);
}

} // namespace AM
