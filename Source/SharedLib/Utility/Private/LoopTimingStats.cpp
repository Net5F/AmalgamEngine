#include "LoopTimingStats.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
LoopTimingStats::LoopTimingStats(std::string_view inDebugName)
: debugName{inDebugName}
, osLatenesses{}
, deadlineLatenesses{}
, iterationCount{0}
, idleIterationCount{0}
, lastReportTime{std::chrono::steady_clock::now()}
{
}

void LoopTimingStats::start()
{
    lastReportTime = std::chrono::steady_clock::now();
}

void LoopTimingStats::recordOsLateness(double latenessS)
{
    osLatenesses.push_back(latenessS);
}

void LoopTimingStats::recordIteration(double timeTillDeadlineS)
{
    iterationCount++;

    // If a deadline is due, record how overdue it is. Otherwise, this
    // iteration is idle.
    double deadlineLatenessS{-timeTillDeadlineS};
    if (deadlineLatenessS >= 0) {
        deadlineLatenesses.push_back(deadlineLatenessS);
    }
    else {
        idleIterationCount++;
    }

    auto now{std::chrono::steady_clock::now()};
    double elapsedS{
        std::chrono::duration<double>(now - lastReportTime).count()};
    if (elapsedS >= REPORT_INTERVAL_S) {
        logStats(elapsedS);
        lastReportTime = now;
    }
}

void LoopTimingStats::logStats(double elapsedS)
{
    std::sort(osLatenesses.begin(), osLatenesses.end());
    std::sort(deadlineLatenesses.begin(), deadlineLatenesses.end());

    LOG_INFO("%s: %u iterations (%.1f/s), %u idle. %zu OS waits, "
             "lateness (ms): p50 %.3f, p90 %.3f, p99 %.3f, max %.3f. "
             "Deadline lateness (ms): p50 %.3f, p90 %.3f, p99 %.3f, max %.3f.",
             debugName.c_str(), iterationCount, (iterationCount / elapsedS),
             idleIterationCount, osLatenesses.size(),
             getPercentile(osLatenesses, .5) * 1000,
             getPercentile(osLatenesses, .9) * 1000,
             getPercentile(osLatenesses, .99) * 1000,
             getPercentile(osLatenesses, 1) * 1000,
             getPercentile(deadlineLatenesses, .5) * 1000,
             getPercentile(deadlineLatenesses, .9) * 1000,
             getPercentile(deadlineLatenesses, .99) * 1000,
             getPercentile(deadlineLatenesses, 1) * 1000);

    iterationCount = 0;
    idleIterationCount = 0;
    osLatenesses.clear();
    deadlineLatenesses.clear();
}

double LoopTimingStats::getPercentile(const std::vector<double>& sortedValues,
                                      double percentile)
{
    if (sortedValues.empty()) {
        return 0;
    }

    std::size_t index{static_cast<std::size_t>(
        percentile * static_cast<double>(sortedValues.size() - 1))};
    return sortedValues[index];
}

} // namespace AM
