#include "PeriodicCallerGroup.h"
#include "PeriodicCaller.h"
#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_timer.h>
#include <algorithm>
#include <chrono>
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
    for (PeriodicCaller* caller : callers) {
        caller->initTimer();
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
        auto sleepStartTime{std::chrono::steady_clock::now()};
        SDL_DelayNS(static_cast<Uint64>(sleepTimeS * SDL_NS_PER_SECOND));
        if (timingStatsEnabled) {
            timingStats.recordOsLateness(
                std::chrono::duration<double>(std::chrono::steady_clock::now()
                                              - sleepStartTime)
                    .count()
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
    double minTimeTillNextCall{std::numeric_limits<double>::max()};
    for (PeriodicCaller* caller : callers) {
        minTimeTillNextCall
            = std::min(minTimeTillNextCall, caller->getTimeTillNextCall());
    }

    return minTimeTillNextCall;
}

bool PeriodicCallerGroup::enoughTimeTillNextCall(double minimumTime)
{
    return (getMinTimeTillNextCall() > minimumTime);
}

} // namespace AM
