#pragma once

#include "ReplicatedComponent.h"

/**
 * This file contains helper functions for working with ReplicatedComponents.
 *
 * We split these out to a separate file because ReplicatedComponents.h is
 * often included in headers, but these are only needed in source files.
 * Keeping them out of headers leads to faster compilation times.
 */
namespace AM
{
class ReplicatedComponentTools
{
public:
    /**
     * Returns the index of the given type within ReplicatedComponentTypes.
     */
    template<typename T>
    static Uint8 asTypeIndex()
    {
        constexpr std::size_t typeIndex{
            boost::mp11::mp_find<ReplicatedComponentTypes, T>::value};
        static_assert(
            typeIndex != boost::mp11::mp_size<ReplicatedComponentTypes>::value,
            "Given type must be present in ReplicatedComponentTypes.");

        return static_cast<Uint8>(typeIndex);
    }

    /**
     * Returns Ts as a vector of indices.
     */
    template<typename... Ts>
    static std::vector<Uint8> asTypeIndices()
    {
        std::vector<Uint8> typeIndices;

        using Types = boost::mp11::mp_list<Ts...>;
        boost::mp11::mp_for_each<Types>([&](auto I) {
            using T = decltype(I);
            typeIndices.push_back(asTypeIndex<T>());
        });

        return typeIndices;
    }

    /**
     * If the given component vector contains the given component types, returns
     * true.
     * If at least 1 component type in Ts is not found, returns false.
     */
    template<typename... Ts>
    static bool
        containsTypes(const std::vector<ReplicatedComponent>& components)
    {
        // Build a list of type indices for each component in the vector.
        std::vector<Uint8> receivedTypes(components.size());
        for (const auto& componentVariant : components) {
            receivedTypes.push_back(
                static_cast<Uint8>(componentVariant.index()));
        }
        std::sort(receivedTypes.begin(), receivedTypes.end());

        // Build a list of type indices for the types we care about.
        std::vector<Uint8> movementTypes{asTypeIndices<Ts...>()};
        std::sort(movementTypes.begin(), movementTypes.end());

        // If the vector contains our desired components.
        if (std::includes(receivedTypes.begin(), receivedTypes.end(),
                          movementTypes.begin(), movementTypes.end())) {
            return true;
        }

        return false;
    }
};

} // End namespace AM
