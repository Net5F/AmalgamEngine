#pragma once

#include "boost/mp11/list.hpp"

namespace AM
{
// Note: Observed components are server-only.
namespace Server
{
/**
 * See EngineReplicatedComponentTypes.h for more info.
 */
using ProjectObservedComponentTypes = boost::mp11::mp_list<>;

} // End namespace Server
} // End namespace AM
