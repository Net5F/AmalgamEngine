#pragma once

#include "NetworkDefs.h"
#include <unordered_map>
#include <memory>

namespace AM
{
namespace Server
{
class Client;

/** A map type used to manage clients. */
using ClientMap = std::unordered_map<NetworkID, std::shared_ptr<Client>>;

} // End namespace Server
} // End namespace AM
