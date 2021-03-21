#include "Configuration.h"
#include <fstream>
#include <iostream>

namespace AM
{

nlohmann::json Configuration::jsonCache;

void Configuration::init(const std::string& runPath)
{
    std::string fileName{"Config.json"};
    std::ifstream fileStream(runPath + fileName);

    if (fileStream.fail()) {
        LOG_ERROR("Failed to open Config.json at path: %s", runPath.c_str());
    }

    try {
        // Parse the config and store the values in jsonCache.
        fileStream >> jsonCache;
    }
    catch(nlohmann::json::parse_error& e) {
        LOG_ERROR("Config.json: %s", e.what());
    }
}

} // End namespace AM
