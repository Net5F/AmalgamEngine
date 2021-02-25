#include "PeriodicCaller.h"
#include "Log.h"

namespace AM
{
PeriodicCaller::PeriodicCaller(std::function<void(void)> inGivenFunct,
                               double inTimestepS, std::string_view inDebugName,
                               bool inSkipLateSteps)
: givenFunct(std::move(inGivenFunct))
, timestepS(inTimestepS)
, debugName(inDebugName)
, skipLateSteps(inSkipLateSteps)
, accumulatedTime(0.0)
, delayedTimeS(-1)
{
    // Prime the timer so we don't get a giant value on the first usage.
    timer.updateSavedTime();
}

void PeriodicCaller::initTimer()
{
    timer.updateSavedTime();
}

void PeriodicCaller::update()
{
    // Accumulate the time passed since last update().
    accumulatedTime += timer.getDeltaSeconds(true);

    // Process as many time steps as have accumulated.
    while (accumulatedTime >= timestepS) {
        // Call the given function.
        givenFunct();

        // Check our execution time.
        double executionTime = timer.getDeltaSeconds(false);
        if (executionTime > timestepS) {
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
    double timeSinceLastCall = timer.getDeltaSeconds(false);
    return (timestepS - (accumulatedTime + timeSinceLastCall));
}

double PeriodicCaller::getProgress()
{
    // Get the time since accumulatedTime was last updated.
    double timeSinceLastCall = timer.getDeltaSeconds(false);
    return ((accumulatedTime + timeSinceLastCall) / timestepS);
}

void PeriodicCaller::reportDelays(double inDelayedTimeS)
{
    delayedTimeS = inDelayedTimeS;
}

} // namespace AM
