#include "UserConfigInitializer.h"
#include "UserConfig.h"

namespace AM
{
namespace Server
{
UserConfigInitializer::UserConfigInitializer()
{
    UserConfig::get();
}

} // End namespace Server
} // End namespace AM
