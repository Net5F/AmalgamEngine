#pragma once

#include "DiscretePosition.h"
#include "HashTools.h"

namespace AM
{
/**
 * A strong type alias, describing the position of a particular spatial
 * partitioning grid cell.
 */
using CellPosition = DiscretePosition<DiscreteImpl::CellTag>;

} // namespace AM

// std::hash() specialization.
namespace std
{
template<>
struct hash<AM::CellPosition> {
    typedef AM::CellPosition argument_type;
    typedef std::size_t result_type;
    result_type operator()(const argument_type& position) const
    {
        std::size_t seed{0};
        AM::hash_combine(seed, position.x);
        AM::hash_combine(seed, position.y);
        return seed;
    }
};
} // namespace std
