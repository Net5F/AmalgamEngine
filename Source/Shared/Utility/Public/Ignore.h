#ifndef IGNORE_H_
#define IGNORE_H_


namespace AM
{

/**
 * Provides a function to suppress "unused parameter" warnings when the
 * unuse is intentional. (Most commonly, when it's a temporary implementation.)
 */
template<class T> void ignore( const T& ) { }

} /* End namespace AM */

#endif /* End IGNORE_H_ */
