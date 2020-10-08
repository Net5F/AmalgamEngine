#pragma once

namespace AM
{
struct MovementComponent {
public:
    static constexpr float DEFAULT_MAX_VEL = 5;

    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    float velX = 0;
    float velY = 0;

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    float maxVelX = DEFAULT_MAX_VEL;
    float maxVelY = DEFAULT_MAX_VEL;
};

template<typename S>
void serialize(S& serializer, MovementComponent& movementComponent)
{
    serializer.value4b(movementComponent.velX);
    serializer.value4b(movementComponent.velY);
}

} // namespace AM
