#pragma once

#include "ConnectionRegistry.h"
#include "SimpleConnection.h"
#include "AccountServiceMessageType.h"

namespace AM
{
namespace AccountServer
{

// TODO: We can start filling this out
/**
 */
class AccountServiceEndpoint
{
public:

private:
    using Connection = SimpleConnection<AccountServiceMessageType>;
    ConnectionRegistry<Connection> connectionRegistry;
};

} // End namespace AccountServer
} // End namespace AM
