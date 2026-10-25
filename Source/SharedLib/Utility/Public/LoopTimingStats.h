#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <vector>

namespace AM
{
/**
 * Records timing stats for a loop that waits until deadlines, and periodically
 * logs them.
 *
 * Used to tune how a loop waits, by showing how late the OS returns from
 * sleeps and how overdue each deadline is when the loop gets to it.
 */
class LoopTimingStats
{
public:
    /**
     * @param inDebugName A name used to identify this loop in the logs.
     */
    LoopTimingStats(std::string_view inDebugName);

    /**
     * Resets the report interval to start at the current time.
     */
    void start();

    /**
     * Records how much later than requested the OS returned from a wait.
     */
    void recordOsLateness(double latenessS);

    /**
     * Records a loop iteration, and logs stats every REPORT_INTERVAL_S.
     *
     * @param timeTillDeadlineS The time until the loop's next deadline at the
     * start of the iteration. If <= 0, records how overdue the deadline is.
     * Else, records an idle iteration.
     */
    void recordIteration(double timeTillDeadlineS);

private:
    /** How often to log timing stats. */
    static constexpr double REPORT_INTERVAL_S{5.0};

    /**
     * Logs the timing stats recorded since the last report, then clears them.
     */
    void logStats(double elapsedS);

    /**
     * Returns the value at the given percentile (0-1) of the given sorted
     * vector.
     */
    double getPercentile(const std::vector<double>& sortedValues,
                         double percentile);

    /** A name used to identify this loop in the logs. */
    const std::string debugName;

    /** How much later (in seconds) than requested the OS returned from each
        sleep, since the last stats report. */
    std::vector<double> osLatenesses;

    /** How overdue (in seconds) the next deadline was at the start of each
        iteration that had a deadline due, since the last stats report. */
    std::vector<double> deadlineLatenesses;

    /** The number of loop iterations since the last stats report. */
    unsigned int iterationCount;

    /** The number of loop iterations since the last stats report that started
        with no deadline due. */
    unsigned int idleIterationCount;

    /** The time that we last logged timing stats. */
    std::chrono::steady_clock::time_point lastReportTime;
};

} // namespace AM
