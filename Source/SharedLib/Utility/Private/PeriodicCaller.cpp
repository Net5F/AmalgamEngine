#include "PeriodicCaller.h"
#include "Timer.h"
#include "Log.h"
#include <cmath>

namespace AM
{
PeriodicCaller::PeriodicCaller(std::function<void(void)> inGivenFunctNoTimestep,
                               double inTimestepS, std::string_view inDebugName,
                               bool inSkipLateSteps)
: givenFunctNoTimestep{std::move(inGivenFunctNoTimestep)}
, timestepS{inTimestepS}
, debugName{inDebugName}
, skipLateSteps{inSkipLateSteps}
, nextCallTime{Timer::getGlobalTime() + inTimestepS}
, delayedTimeS{-1}
{
}

PeriodicCaller::PeriodicCaller(std::function<void(double)> inGivenFunctTimestep,
                               double inTimestepS, std::string_view inDebugName,
                               bool inSkipLateSteps)
: givenFunctTimestep{std::move(inGivenFunctTimestep)}
, timestepS{inTimestepS}
, debugName{inDebugName}
, skipLateSteps{inSkipLateSteps}
, nextCallTime{Timer::getGlobalTime() + inTimestepS}
, delayedTimeS{-1}
{
}

void PeriodicCaller::initTimer(double startTime)
{
    nextCallTime = startTime + timestepS;
}

void PeriodicCaller::update()
{
    // Process as many time steps as are due.
    double currentTime{Timer::getGlobalTime()};
    while (currentTime >= nextCallTime) {
        double callStartTime{Timer::getGlobalTime()};

        // Call whichever function we were given on construction.
        if (givenFunctNoTimestep != nullptr) {
            givenFunctNoTimestep();
        }
        else if (givenFunctTimestep != nullptr) {
            givenFunctTimestep(timestepS);
        }

        // Check our execution time.
        double executionTime{Timer::getGlobalTime() - callStartTime};
        if (executionTime > timestepS) {
            LOG_INFO("%s overran its update timestep. executionTime: %.5fs",
                     debugName.c_str(), executionTime);
        }

        // Move to the next time step and check for delays.
        double delay{currentTime - nextCallTime};
        nextCallTime += timestepS;
        if (currentTime >= nextCallTime) {
            // Update was delayed for longer than timestepS.
            LOG_INFO(
                "Detected a request for multiple %s update calls in the same "
                "frame. Update was delayed by: %.5fs.",
                debugName.c_str(), delay);

            // If we're skipping late steps, skip to the first step after the
            // current time. We stay on our timestep grid, so we stay in phase
            // with any callers that share it.
            if (skipLateSteps) {
                double stepsToSkip{
                    std::floor((currentTime - nextCallTime) / timestepS) + 1};
                nextCallTime += (stepsToSkip * timestepS);
                break;
            }
        }
        else if ((delayedTimeS > 0) && (delay >= delayedTimeS)) {
            // Update was delayed for longer than delayedTimeS.
            LOG_INFO("%s update missed its ideal call time. Update was delayed "
                     "by %.5fs.",
                     debugName.c_str(), delay);
        }
    }
}

double PeriodicCaller::getNextCallTime()
{
    return nextCallTime;
}

double PeriodicCaller::getTimeTillNextCall()
{
    return (nextCallTime - Timer::getGlobalTime());
}

double PeriodicCaller::getProgress()
{
    // Return how far we are into this timestep.
    return (1 - (getTimeTillNextCall() / timestepS));
}

void PeriodicCaller::reportDelays(double inDelayedTimeS)
{
    delayedTimeS = inDelayedTimeS;
}

} // namespace AM
