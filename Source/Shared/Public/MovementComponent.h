#ifndef MOVEMENTCOMPONENT_H_
#define MOVEMENTCOMPONENT_H_


namespace AM
{

struct MovementComponent
{
public:
    static constexpr float DEFAULT_MAX_VEL = 5;

    MovementComponent()
    : velX(0), velY(0), maxVelY(DEFAULT_MAX_VEL), maxVelX(DEFAULT_MAX_VEL)
    {
    }

    /** Current velocities. */
    float velX;
    float velY;
    float maxVelX;
    float maxVelY;
};

} // namespace AM

#endif /* End MOVEMENTCOMPONENT_H_ */
