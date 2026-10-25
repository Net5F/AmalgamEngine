#pragma once

#include "LoopTimingStats.h"
#include <initializer_list>
#include <string_view>
#include <vector>

namespace AM
{
class PeriodicCaller;

/**
 * Manages the timing for a main loop that calls a set of PeriodicCallers.
 *
 * Doesn't call the callers' update() functions. The loop is expected to call
 * those itself, so it can control their order and interleave other work.
 * Instead, this class handles the shared timing logic: initializing the
 * callers' timers, checking how long until the next call, and waiting until
 * it's due.
 */
class PeriodicCallerGroup
{
public:
    /**
     * @param inCallers The loop's callers. Must outlive this object.
     * @param inDebugName A name used to identify this loop in the logs.
     */
    PeriodicCallerGroup(std::initializer_list<PeriodicCaller*> inCallers,
                        std::string_view inDebugName);

    /**
     * Enables recording and periodic logging of the loop's timing stats.
     * Disabled by default.
     */
    void enableTimingStats();

    /**
     * Initializes every caller's timer to the same current time, so callers
     * with compatible timesteps are due at exactly the same times.
     */
    void initTimers();

    /**
     * If timing stats are enabled, records them for a loop iteration. Should
     * be called at the start of each iteration.
     */
    void recordIteration();

    /**
     * Sleeps until shortly before the next caller is due, then spins until
     * it's due.
     */
    void waitForNextCall();

    /**
     * Returns the soonest getTimeTillNextCall() across all callers.
     */
    double getMinTimeTillNextCall();

    /**
     * Returns true if all callers have more than minimumTime left until their
     * next call. Else, false.
     */
    bool enoughTimeTillNextCall(double minimumTime);

private:
    /** How long before the next caller is due that we wake up and start
        spinning. OS sleeps overshoot by a variable amount, so we wake early
        and spin the rest of the way to get called on time.
        Higher value == more accurate timing, but more CPU usage. */
    static constexpr double SPIN_TIME_S{.002};

    /** The callers that this group manages the timing of. */
    std::vector<PeriodicCaller*> callers;

    /** If true, timingStats will be recorded and logged. */
    bool timingStatsEnabled;

    /** Records and logs this loop's timing stats. */
    LoopTimingStats timingStats;
};

} // namespace AM
