#include "UserConfigInitializer.h"
#include "UserConfig.h"

namespace AM
{
namespace WorldServer
{
UserConfigInitializer::UserConfigInitializer()
{
    UserConfig::get();
}

} // End namespace WorldServer
} // End namespace AM
