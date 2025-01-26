#pragma once

#include "ProjectItemPropertyTypes.h"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <variant>

namespace AM
{

/**
 * A variant that holds the proterties that may be attached to an item.
 *
 * Note: Since the engine doesn't have any properties to provide, we can just 
 *       alias the project list. This lets us avoid migration issues without 
 *       having to persist the property lists separately (like we do for 
 *       entity components).
 */
using ItemProperty
    = boost::mp11::mp_rename<ProjectItemPropertyTypes, std::variant>;

// Since all members of a variant are the size of the largest member, we enforce
// a size constraint.
static_assert(sizeof(ItemProperty) <= 64,
              "An item property struct is too large. Please reduce its size to "
              "<= 64B or allocate its fields dynamically.");

} // End namespace AM
