#pragma once

#include <atomic>
#include <cstddef>

namespace AM
{
/** Used to pass data out to the consumer. */
struct NetStatsDump {
    std::size_t bytesSent = 0;
    std::size_t bytesReceived = 0;
};

/**
 * This class is used for tracking relevant network statistics.
 *
 * Note: This is a static class instead of being injected because it would be
 * very inconvenient to pass it from the consuming context down to the producing
 * context.
 *
 * Additionally, this class is tangential to the data flow model of the
 * program. Data enters wherever we're tracking from, and is dumped in either a
 * rendering or logging context.
 */
class NetworkStats
{
public:
    /**
     * Dumps all network stats to the returned object, resetting the current
     * values.
     */
    static NetStatsDump dumpStats();

    // Mutators
    /** Adds inBytesSent to bytesSent. */
    static void recordBytesSent(std::size_t inBytesSent);
    /** Adds inBytesReceived to bytesReceived. */
    static void recordBytesReceived(std::size_t inBytesReceived);

private:
    /** The number of bytes that have been sent since the last dump. */
    static std::atomic<std::size_t> bytesSent;

    /** The number of bytes that have been received since the last dump. */
    static std::atomic<std::size_t> bytesReceived;
};

} // End namespace AM
