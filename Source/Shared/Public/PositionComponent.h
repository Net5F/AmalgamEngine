#ifndef POSITIONCOMPONENT_H_
#define POSITIONCOMPONENT_H_

namespace AM
{

/**
 * Represents an entity's position in the world.
 * When used for rendering a sprite, represents the top left.
 */
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

} // namespace AM

#endif /* End POSITIONCOMPONENT_H_ */
