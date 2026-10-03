#pragma once

#include "NetworkID.h"
#include <unordered_map>
#include <memory>

namespace AM
{
namespace WorldServer
{
class Client;

/** A map type used to manage clients. */
using ClientMap = std::unordered_map<NetworkID, std::shared_ptr<Client>>;

} // End namespace WorldServer
} // End namespace AM
