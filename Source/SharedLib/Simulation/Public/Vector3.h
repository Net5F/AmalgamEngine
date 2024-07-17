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

    Vector3 operator+(const Vector3& other) const;

    Vector3 operator-(const Vector3& other) const;

    Vector3 operator*(const Vector3& other) const;

    Vector3 operator*(float scalar) const;

    Vector3& operator+=(const Vector3& other);

    Vector3& operator-=(const Vector3& other);

    Vector3& operator*=(const Vector3& other);

    bool operator==(const Vector3& other) const;

    bool operator!=(const Vector3& other) const;

    /**
     * Normalizes this vector.
     */
    void normalize();

    /**
     * Returns the dot product of this vector and the given vector.
     */
    float dot(const Vector3& other);

    /**
     * Returns a new vector slid along a plane defined by the given normal.
     * (i.e. returns the component of this vector that lies along the plane)
     */
    Vector3 slide(const Vector3& normal);

    /**
     * Returns the squared distance between this vector and the given
     * vector.
     * We keep it squared to avoid an expensive sqrt. You can use this by
     * squaring the distance you're comparing it to.
     */
    float squaredDistanceTo(const Vector3& other) const;

    /**
     * Prints this box's current values.
     */
    void print() const;
};

template<typename S>
void serialize(S& serializer, Vector3& vector)
{
    serializer.value4b(vector.x);
    serializer.value4b(vector.y);
    serializer.value4b(vector.z);
}

} // namespace AM
