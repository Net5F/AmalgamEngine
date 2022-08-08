#pragma once

#include <string>

/**
 * This file contains structs used in UserConfig.h to combine related
 * config fields.
 */
namespace AM
{
namespace Client
{

struct ServerAddress {
    std::string IP{};

    unsigned int port{};
};

} // End namespace Client
} // End namespace AM
