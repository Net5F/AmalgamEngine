#pragma once

#include <SDL_stdinc.h>
#include "boost/mp11/list.hpp"

namespace AM
{
/** The type of plant that will be grown. */
struct SeedType {
    enum Value : Uint8 { Sunflower, Leek, Pepper };

    Value value{};
};
template<typename S>
void serialize(S& serializer, SeedType& seedType)
{
    serializer.value1b(seedType.value);
}

/** The list of project properties that may be attached to an item. */
using ProjectItemPropertyTypes = boost::mp11::mp_list<SeedType>;

} // End namespace AM
