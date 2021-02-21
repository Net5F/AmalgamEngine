#pragma once

#include "Timer.h"
#include <functional>
#include <string>
#include <string_view>
#include <utility>

/**
 * Name: PeriodicCaller
 * Constraints: derived has tick()
 * Members: timer, timestep
 * Passed in: debugName, timestep, shouldProcessLateTicks
 * Functions: update(), getTimeTillNextIteration()
 */

namespace AM
{

/**
 * A convenience class for calling a function at a particular time step.
 *
 * Useful for time-critical logic that can't afford the delays incurred by
 * using thread sleep/wake mechanisms.
 *
 * Must be fed by calling update() regularly.
 */
class PeriodicCaller
{
public:
    /**
     * See associated members for descriptions.
     */
    PeriodicCaller(std::function<void(void)> inGivenFunct, double inTimestepS,
                   std::string_view inDebugName, bool inSkipLateSteps);

    /**
     * Initializes the timer to the current time.
     * The timer is initialized in the constructor, but this function is
     * useful if you'd like to get all of your callers relatively synchronized.
     */
    void initTimer();

    /**
     * Updates the accumulatedTime. If enough time has passed, calls givenFunct.
     * Note: We explicitly don't forward parameters because this function is
     *       going to be called a ton. Allowing parameters would lead to a
     *       potential performance gotcha if a parameter takes time to acquire.
     */
    void update();

    /**
     * Returns how much time in seconds is left until the next call of
     * givenFunct.
     */
    double getTimeTillNextCall();

    /**
     * Returns how far we are temporally into our wait for the next call.
     * e.g. .1 if we're 10% of the way to the next call.
     */
    double getProgress();

    /**
     * Enables reporting of updates that are late by delayedTimeS or more
     * seconds.
     */
    void reportDelays(double inDelayedTimeS);

private:
    /** The function to call every timestepS seconds. */
    const std::function<void(void)> givenFunct;

    /** The amount of time between calls of givenFunct, in seconds. */
    const double timestepS;

    /** A name used to identify this caller for debug purposes, in case you
        only have a reference to this object and not the function owner. */
    const std::string debugName;

    /**
     * Determines behavior of update() when it detects that multiple time steps
     * have passed since the last time it was called.
     * If true, will only call givenFunct once (and log a warning.)
     * If false, will call givenFunct for each time step (and log warnings.)
     */
    const bool skipLateSteps;

    /** Used to time when we should call givenFunct. */
    Timer timer;

    /** The accumulated time since we last called givenFunct. */
    double accumulatedTime;

    /** An unreasonable amount of time for the update to be late by.
        If <= 0, no delay reporting will occur. */
    double delayedTimeS;
};

} // namespace AM
