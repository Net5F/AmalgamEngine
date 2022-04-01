#include "WorldSinks.h"
#include "WorldSignals.h"

namespace AM
{
namespace Client
{

WorldSinks::WorldSinks(WorldSignals& worldSignals)
: tileMapExtentChanged{worldSignals.tileMapExtentChanged}
{
}

} // End namespace Client
} // End namespace AM
