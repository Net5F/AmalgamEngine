#pragma once

#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "boost/mp11/list.hpp"
#include "boost/mp11/algorithm.hpp"
#include <variant>

namespace AM
{
namespace Server
{

/**
 * See comment in EnginePersistedComponents.h
 */
using PersistedComponentTypes
    = boost::mp11::mp_append<EnginePersistedComponentTypes,
                             ProjectPersistedComponentTypes>;
static_assert(boost::mp11::mp_size<PersistedComponentTypes>::value
                  <= (SDL_MAX_UINT8 + 1),
              "Too many types in PersistedComponentTypes. Max is 256");

/**
 * A variant that holds a persisted component.
 *
 * Used by the server to save entity state to the database.
 */
using PersistedComponent
    = boost::mp11::mp_rename<PersistedComponentTypes, std::variant>;

} // End namespace Server
} // End namespace AM
