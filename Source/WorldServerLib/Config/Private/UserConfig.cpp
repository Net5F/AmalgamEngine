#include "UserConfig.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <string>
#include <fstream>

namespace AM
{
namespace WorldServer
{

UserConfig::UserConfig()
: accountServerIP{}
, accountServerPort{0}
, chatServerIP{}
, chatServerPort{0}
, maxClients{0}
{
    // Open the file.
    std::string fullPath{Paths::BASE_PATH};
    fullPath += "UserConfig.json";
    std::ifstream workingFile(fullPath);
    if (!(workingFile.is_open())) {
        LOG_FATAL("Failed to open UserConfig.json");
    }

    // Parse the file into a json structure.
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(workingFile, nullptr, true, true);
    } catch (nlohmann::json::exception& e) {
        LOG_FATAL("Failed to parse UserConfig.json: %s", e.what());
    }

    // Initialize our members.
    try {
        init(json);
    } catch (nlohmann::json::exception& e) {
        LOG_FATAL("%s", e.what());
    }
}

UserConfig& UserConfig::get()
{
    static UserConfig userConfig;
    return userConfig;
}

UserConfig::ServerAddress UserConfig::getAccountServerAddress()
{
    return {accountServerIP, accountServerPort};
}

void UserConfig::setAccountServerAddress(const ServerAddress& inServerAddress)
{
    accountServerIP = inServerAddress.IP;
    accountServerPort = inServerAddress.port;
}

UserConfig::ServerAddress UserConfig::getChatServerAddress()
{
    return {chatServerIP, chatServerPort};
}

void UserConfig::setChatServerAddress(const ServerAddress& inServerAddress)
{
    chatServerIP = inServerAddress.IP;
    chatServerPort = inServerAddress.port;
}

unsigned int UserConfig::getMaxClients()
{
    return maxClients;
}

void UserConfig::setMaxClients(unsigned int inMaxClients)
{
    if (inMaxClients == 0) {
        LOG_FATAL("Invalid maxClients value: %u. Must be at least 1.",
                  inMaxClients);
    }
    maxClients = inMaxClients;
}

void UserConfig::init(nlohmann::json& json)
{
    // AccountServer address.
    setAccountServerAddress(
        {json.at("accountServerIP"), json.at("accountServerPort")});

    // ChatServer address.
    setChatServerAddress({json.at("chatServerIP"), json.at("chatServerPort")});

    // Max clients.
    setMaxClients(json.at("maxClients"));
}

} // End namespace WorldServer
} // End namespace AM
