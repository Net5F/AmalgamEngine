#include "Vector3.h"
#include "Log.h"

namespace AM
{
Vector3 Vector3::operator+(const Vector3& other) const
{
    return {(x + other.x), (y + other.y), (z + other.z)};
}

Vector3 Vector3::operator-(const Vector3& other) const
{
    return {(x - other.x), (y - other.y), (z - other.z)};
}

Vector3 Vector3::operator*(const Vector3& other) const
{
    return {(x * other.x), (y * other.y), (z * other.z)};
}

Vector3 Vector3::operator*(float scalar) const
{
    return {(x * scalar), (y * scalar), (z * scalar)};
}

Vector3& Vector3::operator+=(const Vector3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3& Vector3::operator*=(const Vector3& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

Vector3& Vector3::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3& Vector3::operator/=(const Vector3& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

Vector3& Vector3::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

bool Vector3::operator==(const Vector3& other) const
{
    return (x == other.x) && (y == other.y) && (z == other.z);
}

bool Vector3::operator!=(const Vector3& other) const
{
    return (x != other.x) || (y != other.y) || (z != other.z);
}

void Vector3::normalize()
{
    const float length{std::sqrt((x * x) + (y * y) + (z * z))};
    *this /= length;
}

float Vector3::dot(const Vector3& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

Vector3 Vector3::slide(const Vector3& normal) const
{
    return *this - (normal * dot(normal));
}

Vector3 Vector3::moveTowards(const Vector3& otherPoint, float distance) const
{
    float squaredDistanceToOther{squaredDistanceTo(otherPoint)};
    float squaredDistance{distance * distance};

    // If we can reach it, return otherPoint.
    if (squaredDistance >= squaredDistanceToOther) {
        return otherPoint;
    }

    // Can't reach. Calculate the intermediate point.
    Vector3 vector{otherPoint - *this};
    vector.normalize();
    vector *= distance;
    return *this + vector;
}

float Vector3::squaredDistanceTo(const Vector3& other) const
{
    Vector3 distance{std::abs(x - other.x), std::abs(y - other.y),
                     std::abs(z - other.z)};
    return {(distance.x * distance.x) + (distance.y * distance.y)
            + (distance.z * distance.z)};
}

void Vector3::print() const
{
    LOG_INFO("(%.4f, %.4f, %.4f)", x, y, z);
}

} // End namespace AM
