#pragma once

#include "Log.h"
#include "nlohmann/json.hpp"
#include <string>

namespace AM
{

/**
 * Static class. Parses Config.json and provides the values at runtime.
 */
class Configuration
{
public:
    /**
     * @param runPath  The path that the executable was started from. Used
     *                 when building the Config.json file path.
     */
    static void init(const std::string& runPath);

    template<typename T>
    static T get(const std::string& key) {
        try {
            return jsonCache[key];
        }
        catch(nlohmann::json::type_error& e) {
            LOG_ERROR("%s", e.what());
        }
    }

private:
    static nlohmann::json jsonCache;
};

} // End namespace AM
