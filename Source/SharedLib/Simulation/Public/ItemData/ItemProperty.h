#pragma once

#include "EngineItemPropertyTypes.h"
#include "ProjectItemPropertyTypes.h"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <variant>

namespace AM
{

/**
 * See comment in EngineItemPropertyTypes.h
 */
using ItemPropertyTypes
    = boost::mp11::mp_append<EngineItemPropertyTypes,
                             ProjectItemPropertyTypes>;

/**
 * A variant that holds the proterties that may be attached to an item.
 */
using ItemProperty 
    = boost::mp11::mp_rename<ItemPropertyTypes, std::variant>;

} // End namespace AM
