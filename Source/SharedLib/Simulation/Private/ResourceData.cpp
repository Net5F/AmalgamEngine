#include "ResourceData.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <fstream>

namespace AM
{

ResourceData::ResourceData()
{
    // Open the file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "ResourceData.json";
    std::ifstream workingFile(fullPath);
    if (!(workingFile.is_open())) {
        LOG_FATAL("Failed to open ResourceData.json");
    }

    // Parse the file into a json structure.
    try {
        resourceDataJson = std::make_unique<nlohmann::json>(
            nlohmann::json::parse(workingFile, nullptr, true));
    } catch (nlohmann::json::exception& e) {
        LOG_FATAL("Failed to parse ResourceData.json: %s", e.what());
    }

}

ResourceData::~ResourceData() = default;

nlohmann::json& ResourceData::get()
{
    if (!resourceDataJson) {
        LOG_FATAL("Tried to get uninitialized json object.");
    }

    return *resourceDataJson;
}

} // End namespace AM
