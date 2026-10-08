#pragma once

#include "nlohmann/json_fwd.hpp"

namespace AM
{
namespace AccountServer
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
    unsigned int getDatabaseWorkerCount();
    void setDatabaseWorkerCount(unsigned int inDatabaseWorkerCount);

private:
    /**
     * Initializes our members using the given json.
     * @throw nlohmann::json::exception if an expected field is not found.
     */
    void init(nlohmann::json& json);

    unsigned int databaseWorkerCount;
};

} // End namespace AccountServer
} // End namespace AM
