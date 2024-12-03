#include "PeriodicCaller.h"
#include "Log.h"

namespace AM
{
PeriodicCaller::PeriodicCaller(std::function<void(void)> inGivenFunctNoTimestep,
                               double inTimestepS, std::string_view inDebugName,
                               bool inSkipLateSteps)
: givenFunctNoTimestep{std::move(inGivenFunctNoTimestep)}
, timestepS{inTimestepS}
, debugName{inDebugName}
, skipLateSteps{inSkipLateSteps}
, timer{}
, accumulatedTime{0.0}
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
, timer{}
, accumulatedTime{0.0}
, delayedTimeS{-1}
{
}

void PeriodicCaller::initTimer()
{
    timer.reset();
}

void PeriodicCaller::update()
{
    // Accumulate the time passed since the last update().
    accumulatedTime += timer.getTimeAndReset();

    // Process as many time steps as have accumulated.
    while (accumulatedTime >= timestepS) {
        double updateStartTime{timer.getTime()};

        // Call whichever function we were given on construction.
        if (givenFunctNoTimestep != nullptr) {
            givenFunctNoTimestep();
        }
        else if (givenFunctTimestep != nullptr) {
            givenFunctTimestep(timestepS);
        }

        // Check our execution time.
        double executionTime{timer.getTime()};
        if ((executionTime - updateStartTime) > timestepS) {
            LOG_INFO("%s overran its update timestep. executionTime: %.5fs",
                     debugName.c_str(), executionTime);
        }

        // Deduct this time step from the accumulator and check for delays.
        accumulatedTime -= timestepS;
        if (accumulatedTime >= timestepS) {
            // Update was delayed for longer than timestepS.
            LOG_INFO(
                "Detected a request for multiple %s update calls in the same "
                "frame. Update was delayed by: %.5fs.",
                debugName.c_str(), accumulatedTime);

            // If we're skipping late steps, reset to a fresh state.
            if (skipLateSteps) {
                accumulatedTime = 0;
                break;
            }
        }
        else if ((delayedTimeS > 0) && (accumulatedTime >= delayedTimeS)) {
            // Update was delayed for longer than delayedTimeS.
            LOG_INFO("%s update missed its ideal call time. Update was delayed "
                     "by %.5fs.",
                     debugName.c_str(), accumulatedTime);
        }
    }
}

double PeriodicCaller::getTimeTillNextCall()
{
    // Get the time since accumulatedTime was last updated.
    double timeSinceLastCall{timer.getTime()};

    // Return the amount of time until our next call.
    return (timestepS - (accumulatedTime + timeSinceLastCall));
}

double PeriodicCaller::getProgress()
{
    // Get the time since accumulatedTime was last updated.
    double timeSinceLastCall{timer.getTime()};

    // Return how far we are into this timestep.
    return ((accumulatedTime + timeSinceLastCall) / timestepS);
}

void PeriodicCaller::reportDelays(double inDelayedTimeS)
{
    delayedTimeS = inDelayedTimeS;
}

} // namespace AM
