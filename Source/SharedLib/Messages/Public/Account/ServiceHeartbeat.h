#pragma once

#include "AccountServiceMessageType.h"

namespace AM
{

/**
 * Sent periodically by a service to keep its connection to the Account Server
 * open, and to detect if the Account Server has gone away.
 *
 * The Account Server responds with a ServiceHeartbeatResponse.
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
