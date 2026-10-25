#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <utility>

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
    PeriodicCaller(std::function<void(void)> inGivenFunctNoTimestep,
                   double inTimestepS, std::string_view inDebugName,
                   bool inSkipLateSteps);

    /**
     * Overload for functions that take a timestep argument.
     * The given function will be passed the timestep in seconds.
     */
    PeriodicCaller(std::function<void(double)> inGivenFunctTimestep,
                   double inTimestepS, std::string_view inDebugName,
                   bool inSkipLateSteps);

    /**
     * Sets the next call to be one timestep from the given time, in
     * Timer::getGlobalTime() seconds.
     * This is done with the current time in the constructor, but this
     * function is useful for restarting the timer. Giving multiple callers the
     * same startTime puts them exactly in phase, so callers with compatible
     * timesteps will be due at the same times.
     */
    void initTimer(double startTime);

    /**
     * Calls givenFunct for each time step that is due.
     * Note: We explicitly don't forward parameters because this function is
     *       going to be called a ton. Allowing parameters would lead to a
     *       potential performance gotcha if a parameter takes time to acquire.
     */
    void update();

    /**
     * Returns the time, in Timer::getGlobalTime() seconds, that givenFunct is
     * next due to be called.
     */
    double getNextCallTime();

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
    /** The function to call every timestepS seconds, if given a callback with
        no arguments. */
    const std::function<void(void)> givenFunctNoTimestep;

    /** The function to call every timestepS seconds, if given a callback that
        takes an argument of the time passed. */
    const std::function<void(double)> givenFunctTimestep;

    /** The amount of time between calls of givenFunct, in seconds. */
    const double timestepS;

    /** A name used to identify this caller for debug purposes, in case you
        only have a reference to this object and not the function owner. */
    const std::string debugName;

    /**
     * Determines behavior of update() when it detects that multiple time steps
     * have passed since the last time it was called.
     * If true, will only call givenFunct once (and log a warning).
     * If false, will call givenFunct for each time step (and log warnings).
     */
    const bool skipLateSteps;

    /** The time, in Timer::getGlobalTime() seconds, that givenFunct is next
        due to be called. */
    double nextCallTime;

    /** An unreasonable amount of time for the update to be late by.
        If <= 0, no delay reporting will occur. */
    double delayedTimeS;
};

} // namespace AM
