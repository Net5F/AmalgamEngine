#ifndef POSITIONCOMPONENT_H_
#define POSITIONCOMPONENT_H_

namespace AM
{

struct PositionComponent
{
public:
    PositionComponent()
    : x(0), y(0)
    {
    }

    /** Current position. */
    float x;
    float y;
};

} /* End namespace NW */

#endif /* End POSITIONCOMPONENT_H_ */
