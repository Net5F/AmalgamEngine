#pragma once

#include "boost/mp11/list.hpp"

namespace AM
{
// Note: Observed components are server-only.
namespace Server
{
/**
 * All of the engine's component types that should be observed and auto-
 * replicated. When an observed component is updated (using patch() or
 * replace()), an Update message containing the component will be sent by the
 * server to all nearby clients.
 *
 * Note: Every type in this list must also be in ProjectReplicatedComponentTypes.
 * Note: For performance reasons, if something is going to be updated very
 *       frequently (e.g. movement), consider manually handling it in a system
 *       instead of adding it here.
 */
using ProjectObservedComponentTypes = boost::mp11::mp_list<>;

} // End namespace Server
} // End namespace AM
