#include "UserConfigInitializer.h"
#include "UserConfig.h"

namespace AM
{
namespace Client
{
UserConfigInitializer::UserConfigInitializer()
{
    UserConfig::get();
}

} // End namespace Client
} // End namespace AM
