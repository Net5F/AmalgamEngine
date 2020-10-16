#pragma once

#include <atomic>

namespace AM
{

/** Used to pass data out to the consumer. */
struct NetStatsDump {
    unsigned int bytesSent = 0;
    unsigned int bytesReceived = 0;
};

/**
 * This class is used for tracking relevant network statistics.
 *
 * Note: This is a static class instead of being injected because it would be very inconvenient
 *       to pass it from the consuming context down to the producing context.
 *
 *       Additionally, this class is tangential to the data flow model of the program.
 *       Data enters wherever we're tracking from, and is dumped in either a rendering or
 *       logging context.
 */
class NetworkStats
{
public:
    /**
     * Dumps all network stats to the returned object, resetting the current values.
     */
    static NetStatsDump dumpStats();

    // Mutators
    /** Adds inBytesSent to bytesSent. */
    static void recordBytesSent(unsigned int inBytesSent);
    /** Adds inBytesReceived to bytesReceived. */
    static void recordBytesReceived(unsigned int inBytesReceived);

private:
    /** The number of bytes that have been sent since the last dump. */
    static std::atomic<unsigned int> bytesSent;

    /** The number of bytes that have been received since the last dump. */
    static std::atomic<unsigned int> bytesReceived;
};

} // End namespace AM
