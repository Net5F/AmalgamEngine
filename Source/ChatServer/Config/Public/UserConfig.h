#pragma once

#include "nlohmann/json_fwd.hpp"
#include <string>

namespace AM
{
namespace ChatServer
{

/**
 * A singleton instance that loads the user-defined configuration from
 * UserConfig.json into memory and provides an interface for accessing and
 * modifying it.
 *
 * Note: This class expects a UserConfig.json file to be present in the same
 *       directory as the application executable.
 */
class UserConfig
{
public:
    /**
     * @pre SDL must be initialized.
     */
    UserConfig();

    /**
     * Returns the singleton instance.
     */
    static UserConfig& get();

    //-------------------------------------------------------------------------
    // Configuration Interface
    //-------------------------------------------------------------------------
    struct ServerAddress {
        std::string IP{};
        unsigned int port{};
    };
    ServerAddress getAccountServerAddress();
    void setAccountServerAddress(const ServerAddress& inServerAddress);

private:
    /**
     * Initializes our members using the given json.
     * @throw nlohmann::json::exception if an expected field is not found.
     */
    void init(nlohmann::json& json);

    /** AccountServer's IP address and service port. */
    std::string accountServerIP;
    unsigned int accountServerPort;
};

} // End namespace ChatServer
} // End namespace AM
