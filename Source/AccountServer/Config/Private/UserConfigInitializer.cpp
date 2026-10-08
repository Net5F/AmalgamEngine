#include "UserConfigInitializer.h"
#include "UserConfig.h"

namespace AM
{
namespace AccountServer
{
UserConfigInitializer::UserConfigInitializer()
{
    UserConfig::get();
}

} // End namespace AccountServer
} // End namespace AM
