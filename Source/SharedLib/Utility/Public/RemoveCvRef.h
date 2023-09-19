#pragma once

namespace AM
{

/**
 * Removes const, volatile, and reference qualifiers from a type.
 * Note: If the STL ever provides this, we should use that instead.
 */
template<class T>
using remove_cv_ref =
    typename std::remove_cv<typename std::remove_reference<T>::type>::type;

} // End namespace AM
