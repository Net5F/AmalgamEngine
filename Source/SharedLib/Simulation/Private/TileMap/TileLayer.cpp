#include "TileLayer.h"
#include "GraphicSets.h"
#include "AMAssert.h"

namespace AM
{

GraphicRef TileLayer::getGraphic() const
{
    // Note: We don't need to check if the slots refer to a non-null 
    //       graphic, because the null graphic is fine to return.
    if (type == Type::Floor) {
        return static_cast<const FloorGraphicSet&>(graphicSet.get())
            .graphic;
    }
    else if (type == Type::FloorCovering) {
        return static_cast<const FloorCoveringGraphicSet&>(graphicSet.get())
            .graphics[graphicIndex];
    }
    else if (type == Type::Wall) {
        return static_cast<const WallGraphicSet&>(graphicSet.get())
            .graphics[graphicIndex];
    }
    else {
        // Type::Object
        return static_cast<const ObjectGraphicSet&>(graphicSet.get())
            .graphics[graphicIndex];
    }
}

} // End namespace AM
