#include "UserConfigInitializer.h"
#include "UserConfig.h"

namespace AM
{
namespace ChatServer
{
UserConfigInitializer::UserConfigInitializer()
{
    UserConfig::get();
}

} // End namespace ChatServer
} // End namespace AM
