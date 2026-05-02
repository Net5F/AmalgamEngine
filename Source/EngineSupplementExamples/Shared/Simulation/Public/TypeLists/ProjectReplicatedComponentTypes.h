#pragma once

#include "boost/mp11/list.hpp"

namespace AM
{
/**
 * See EngineReplicatedComponentTypes.h for more info.
 */
using ProjectSelfInitComponentTypes = boost::mp11::mp_list<>;

using ProjectSelfUpdateComponentTypes = boost::mp11::mp_list<>;

using ProjectInRangeInitComponentTypes = boost::mp11::mp_list<>;

using ProjectInRangeUpdateComponentTypes = boost::mp11::mp_list<>;

} // End namespace AM
