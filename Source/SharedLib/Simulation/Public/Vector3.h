#pragma once

#include <cmath>

namespace AM
{
/**
 * A 3D vector, usually in world space.
 *
 * When representing a point in world space, we often use the Position class.
 * May also represent a direction.
 */
struct Vector3 {
    float x{0};
    float y{0};
    float z{0};

    Vector3 operator+(const Vector3& other) const
    {
        return {(x + other.x), (y + other.y), (z + other.z)};
    }

    Vector3 operator-(const Vector3& other) const
    {
        return {(x - other.x), (y - other.y), (z - other.z)};
    }

    Vector3 operator*(const Vector3& other) const
    {
        return {(x * other.x), (y * other.y), (z * other.z)};
    }

    Vector3& operator+=(const Vector3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& operator*=(const Vector3& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    bool operator==(const Vector3& other) const
    {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }

    bool operator!=(const Vector3& other) const
    {
        return (x != other.x) || (y != other.y) || (z != other.z);
    }

    /**
     * Returns the squared distance between this vector and the given
     * vector.
     * We keep it squared to avoid an expensive sqrt. You can use this by
     * squaring the distance you're comparing it to.
     */
    float squaredDistanceTo(const Vector3& other) const
    {
        Vector3 distance{std::abs(x - other.x), std::abs(y - other.y),
                         std::abs(z - other.z)};
        return {(distance.x * distance.x) + (distance.y * distance.y)
                + (distance.z * distance.z)};
    }
};

template<typename S>
void serialize(S& serializer, Vector3& vector)
{
    serializer.value4b(vector.x);
    serializer.value4b(vector.y);
    serializer.value4b(vector.z);
}

} // namespace AM
