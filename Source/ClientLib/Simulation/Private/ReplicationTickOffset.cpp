#include "ReplicationTickOffset.h"
#include "Log.h"

namespace AM
{
namespace Client
{

void ReplicationTickOffset::applyAdjustment(int adjustment)
{
    // We set our client ahead of the server by an amount equal to our latency,
    // but this means that received messages will appear to be doubly far into
    // the past.
    // To account for this, we double the adjustment before applying.
    // We also negate it since we're reversing the direction.
    offset += (-2 * adjustment);

    if (offset >= 0) {
        LOG_FATAL("Adjusted replication tick offset too far into the "
                  "future. offset: %u",
                  offset);
    }
}

int ReplicationTickOffset::get()
{
    return offset;
}

} // End namespace Client
} // End namespace AM
