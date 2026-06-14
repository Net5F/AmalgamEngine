#include "AMMath.h"
#include <cmath>

namespace AM
{

Vector3 Math::min(const Vector3& a, const Vector3& b)
{
    Vector3 res;
    res.x = std::min(a.x, b.x);
    res.y = std::min(a.y, b.y);
    res.z = std::min(a.z, b.z);
    return res;
}

Vector3 Math::max(const Vector3& a, const Vector3& b)
{
    Vector3 res;
    res.x = std::max(a.x, b.x);
    res.y = std::max(a.y, b.y);
    res.z = std::max(a.z, b.z);
    return res;
}

Vector3 Math::abs(const Vector3& vector)
{
    return {std::abs(vector.x), std::abs(vector.y), std::abs(vector.z)};
}

bool Math::isEqualApproxWorld(float a, float b)
{
    // Check for exact equality (required to handle "infinity" values).
    if (a == b) {
        return true;
    }

    return std::abs(a - b) < WORLD_EPSILON;
}

bool Math::isEqualApproxGeneric(float a, float b)
{
    // Note: This logic was copied from Godot.

    // Check for exact equality (required to handle "infinity" values).
    if (a == b) {
        return true;
    }

    // Scale our epsilon to match the given value's scale.
    float tolerance{GENERIC_EPSILON * std::abs(a)};
    if (tolerance < GENERIC_EPSILON) {
        tolerance = GENERIC_EPSILON;
    }

    return std::abs(a - b) < tolerance;
}

float Math::inverseLerp(float a, float b, float x)
{
    return (x - a) / (b - a);
}

float Math::smoothstep(float edge0, float edge1, float x)
{
    // Scale and clamp
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);

    return x * x * (3.0f - 2.0f * x);
}

} // End namespace AM
