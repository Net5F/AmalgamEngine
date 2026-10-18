#pragma once

#include "AccountServiceMessageType.h"

namespace AM
{

/**
 * Sent periodically by a service to keep its connection to AccountServer
 * open, and to detect if AccountServer has gone away.
 *
 * AccountServer responds with a ServiceHeartbeatResponse.
 */
struct ServiceHeartbeat {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ServiceHeartbeat};
};

template<typename S>
void serialize(S&, ServiceHeartbeat&)
{
}

} // End namespace AM
