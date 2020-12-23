#pragma once

namespace AM
{
struct Movement {
public:
    static constexpr float DEFAULT_MAX_VEL = 5;

    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    float velX{0};
    float velY{0};

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    float maxVelX{DEFAULT_MAX_VEL};
    float maxVelY{DEFAULT_MAX_VEL};
};

template<typename S>
void serialize(S& serializer, Movement& movement)
{
    serializer.value4b(movement.velX);
    serializer.value4b(movement.velY);
}

} // namespace AM
