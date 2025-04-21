#include "Castable.h"
#include "SharedConfig.h"

namespace AM
{

Cylinder Castable::getTargetCylinder(const Vector3& position) const
{
    static constexpr float HALF_HEIGHT{
        SharedConfig::CAST_TARGET_CYLINDER_HALF_HEIGHT};

    Cylinder targetCylinder{position, radius, HALF_HEIGHT};
    targetCylinder.center.z -= HALF_HEIGHT;

    return targetCylinder;
}

bool Castable::hasVisuals() const
{
    bool hasCastingGraphic{castingGraphicType != EntityGraphicType::NotSet};
    bool hasCastCompleteGraphic{castCompleteGraphicType
                                != EntityGraphicType::NotSet};
    bool hasVisualEffect{castCompleteVisualEffects.size() != 0};
    bool hasAVEntity{castCompleteAVEntities.size() != 0};

    return hasCastingGraphic || hasCastCompleteGraphic || hasVisualEffect
           || hasAVEntity;
}

} // End namespace AM
