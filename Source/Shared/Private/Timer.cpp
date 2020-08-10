#include <Timer.h>
#include "Debug.h"

namespace AM
{

const Uint64& Timer::COUNT_PER_SECOND() {
    static const Uint64 COUNT_PER_SECOND = SDL_GetPerformanceFrequency();
    return COUNT_PER_SECOND;
}

Timer::Timer()
: savedCount(0)
{
}

Uint64 Timer::getDeltaCount(bool updateSavedTime)
{
    Uint64 currentCount = SDL_GetPerformanceCounter();
    Uint64 deltaCount =  currentCount - savedCount;

    if (updateSavedTime) {
        savedCount = currentCount;
    }

    return deltaCount;
}

void Timer::updateSavedCount()
{
    savedCount = SDL_GetPerformanceCounter();
}

double Timer::countToSeconds(Uint64 count) {
    return count / static_cast<double>(COUNT_PER_SECOND());
}

Uint64 Timer::secondsToCount(double seconds) {
    return COUNT_PER_SECOND() * seconds;
}

Uint64 Timer::ipsToCount(unsigned int iterationsPerSecond) {
    return COUNT_PER_SECOND() / iterationsPerSecond;
}

} // namespace AM
