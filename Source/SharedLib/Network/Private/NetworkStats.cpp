#include "NetworkStats.h"

namespace AM
{
// Initialize data.
std::atomic<std::size_t> NetworkStats::bytesSent = 0;
std::atomic<std::size_t> NetworkStats::bytesReceived = 0;

NetStatsDump NetworkStats::dumpStats()
{
    // Fill the dump object while resetting the tracked values.
    NetStatsDump netStatsDump;

    netStatsDump.bytesSent = bytesSent.exchange(0);
    netStatsDump.bytesReceived = bytesReceived.exchange(0);

    return netStatsDump;
}

void NetworkStats::recordBytesSent(std::size_t inBytesSent)
{
    bytesSent += inBytesSent;
}

void NetworkStats::recordBytesReceived(std::size_t inBytesReceived)
{
    bytesReceived += inBytesReceived;
}

} // End namespace AM
