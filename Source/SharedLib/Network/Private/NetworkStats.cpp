#include "NetworkStats.h"

namespace AM
{
// Initialize data.
std::atomic<unsigned int> NetworkStats::bytesSent = 0;
std::atomic<unsigned int> NetworkStats::bytesReceived = 0;

NetStatsDump NetworkStats::dumpStats()
{
    // Fill the dump object while resetting the tracked values.
    NetStatsDump netStatsDump;

    netStatsDump.bytesSent = bytesSent.exchange(0);
    netStatsDump.bytesReceived = bytesReceived.exchange(0);

    return netStatsDump;
}

void NetworkStats::recordBytesSent(unsigned int inBytesSent)
{
    bytesSent += inBytesSent;
}

void NetworkStats::recordBytesReceived(unsigned int inBytesReceived)
{
    bytesReceived += inBytesReceived;
}

} // End namespace AM
