#pragma once

#include "boost/mp11/list.hpp"

namespace AM
{
// Note: AI is server-only.
namespace WorldServer
{
/**
 * Add AI classes to this list to have them be processed by the engine.
 *
 * Note: Every type in this list must be derived from AILogic.
 */
using ProjectAITypes = boost::mp11::mp_list<>;

} // End namespace WorldServer
} // End namespace AM
