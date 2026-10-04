#pragma once

#include "AccountServiceMessageType.h"

namespace AM
{

/**
 * Sent by the Account Server in response to a ServiceHeartbeat.
 */
struct ServiceHeartbeatResponse {
    static constexpr AccountServiceMessageType MESSAGE_TYPE{
        AccountServiceMessageType::ServiceHeartbeatResponse};
};

template<typename S>
void serialize(S&, ServiceHeartbeatResponse&)
{
}

} // End namespace AM
