#pragma once

namespace AM
{
/**
 * Represents the velocity of something in the world.
 *
 * Velocity is always in terms of world units per second.
 *
 * Note: This isn't a component that we attach to entities. For the related 
 *       component, see Movement.h.
 */
struct Velocity {
    /** Current velocity. */
    float x{0};
    float y{0};
    float z{0};

    Velocity operator+(const Velocity& other) const
    {
        return {(x + other.x), (y + other.y), (z + other.z)};
    }

    Velocity& operator+=(const Velocity& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
};

template<typename S>
void serialize(S& serializer, Velocity& velocity)
{
    serializer.value4b(velocity.x);
    serializer.value4b(velocity.y);
    serializer.value4b(velocity.z);
}

} // namespace AM
