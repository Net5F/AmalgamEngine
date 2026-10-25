#include "PeriodicCallerGroup.h"
#include "PeriodicCaller.h"
#include "Timer.h"
#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <limits>

namespace AM
{
PeriodicCallerGroup::PeriodicCallerGroup(
    std::initializer_list<PeriodicCaller*> inCallers,
    std::string_view inDebugName)
: callers{inCallers}
, timingStatsEnabled{false}
, timingStats{inDebugName}
{
}

void PeriodicCallerGroup::enableTimingStats()
{
    timingStatsEnabled = true;
    timingStats.start();
}

void PeriodicCallerGroup::initTimers()
{
    // Start every caller from the same time, so callers with compatible
    // timesteps are due at exactly the same times.
    double startTime{Timer::getGlobalTime()};
    for (PeriodicCaller* caller : callers) {
        caller->initTimer(startTime);
    }

    timingStats.start();
}

void PeriodicCallerGroup::recordIteration()
{
    if (timingStatsEnabled) {
        timingStats.recordIteration(getMinTimeTillNextCall());
    }
}

void PeriodicCallerGroup::waitForNextCall()
{
    // If we have enough time, sleep until shortly before the next caller is
    // due.
    double timeTillNextCall{getMinTimeTillNextCall()};
    if (timeTillNextCall > SPIN_TIME_S) {
        double sleepTimeS{timeTillNextCall - SPIN_TIME_S};
        double sleepStartTime{Timer::getGlobalTime()};
        SDL_DelayNS(static_cast<Uint64>(sleepTimeS * SDL_NS_PER_SECOND));
        if (timingStatsEnabled) {
            timingStats.recordOsLateness(Timer::getGlobalTime() - sleepStartTime
                                         - sleepTimeS);
        }
    }

    // Spin until the next caller is due.
    while (getMinTimeTillNextCall() > 0) {
        SDL_CPUPauseInstruction();
    }
}

double PeriodicCallerGroup::getMinTimeTillNextCall()
{
    double minNextCallTime{std::numeric_limits<double>::max()};
    for (PeriodicCaller* caller : callers) {
        minNextCallTime = std::min(minNextCallTime, caller->getNextCallTime());
    }

    return (minNextCallTime - Timer::getGlobalTime());
}

bool PeriodicCallerGroup::enoughTimeTillNextCall(double minimumTime)
{
    return (getMinTimeTillNextCall() > minimumTime);
}

} // namespace AM
