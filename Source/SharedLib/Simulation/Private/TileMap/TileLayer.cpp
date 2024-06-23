#include "TileLayer.h"
#include "GraphicSets.h"
#include "AMAssert.h"

namespace AM
{

GraphicRef TileLayer::getGraphic() const
{
    return getGraphic(type, graphicSet, graphicValue);
}

GraphicRef TileLayer::getGraphic(Type type, const GraphicSet& graphicSet,
                                 Uint8 graphicValue)
{
    // Note: We don't need to check if the slots refer to a non-null 
    //       graphic, because the null graphic is fine to return.
    if (type == Type::Terrain) {
        Terrain::Height height{Terrain::getHeight(graphicValue)};
        return static_cast<const TerrainGraphicSet&>(graphicSet)
            .graphics[height];
    }
    else if (type == Type::Floor) {
        return static_cast<const FloorGraphicSet&>(graphicSet)
            .graphics[graphicValue];
    }
    else if (type == Type::Wall) {
        return static_cast<const WallGraphicSet&>(graphicSet)
            .graphics[graphicValue];
    }
    else {
        // Type::Object
        return static_cast<const ObjectGraphicSet&>(graphicSet)
            .graphics[graphicValue];
    }
}

} // End namespace AM
