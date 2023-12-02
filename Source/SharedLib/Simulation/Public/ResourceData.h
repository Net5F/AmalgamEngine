#pragma once

#include <memory>
#include "nlohmann/json_fwd.hpp"

namespace AM
{

/**
 * Parses ResourceData.json into an in-memory object so we can pass it to 
 * SpriteData, IconData, etc.
 *
 * Note: This class expects a ResourceData.json file to be present in the same
 *       directory as the application executable.
 */
class ResourceData {
public:
    /**
     * Parses ResourceData.json. Errors if it fails to parse.
     */
    ResourceData();

    ~ResourceData();

    /**
     * Returns the parsed json object.
     */
    nlohmann::json& get();

private:
    // Note: We use a pointer so we can forward declare.
    std::unique_ptr<nlohmann::json> resourceDataJson;
};

} // End namespace AM
