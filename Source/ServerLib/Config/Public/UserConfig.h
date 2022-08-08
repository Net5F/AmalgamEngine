#pragma once

#include "nlohmann/json_fwd.hpp"

namespace AM
{
namespace Server
{
/**
 * A singleton instance that loads the user-defined configuration from
 * UserConfig.json into memory and provides an interface for accessing and
 * modifying it.
 *
 * Note: This class expects a UserConfig.json file to be present in the same
 *       directory as the application executable.
 *
 * TODO: If we eventually decide to live-refresh some of these fields, we can
 *       add signals that get emitted on change.
 *       E.g.:
 *           Public
 *               entt::sigh<void(ScreenRect)> windowSizeChanged;
 *           Private
 *               entt::sink<void(ScreenRect)> windowSizeSink;
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

private:
    /**
     * Initializes our members using the given json.
     * @throw nlohmann::json::exception if an expected field is not found.
     */
    void init(nlohmann::json& json);
};

} // End namespace Server
} // End namespace AM
