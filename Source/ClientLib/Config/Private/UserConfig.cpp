#include "UserConfig.h"
#include "Paths.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <SDL.h>
#include <string>
#include <fstream>

namespace AM
{
namespace Client
{

UserConfig::UserConfig()
: fullscreenMode{}
, windowSizeWidth{}
, windowSizeHeight{}
, framesPerSecond{}
, frameTimestepS{}
, serverIP{}
, serverPort{}
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

unsigned int UserConfig::getFullscreenMode()
{
    return fullscreenMode;
}

void UserConfig::setFullscreenMode(unsigned int inFullscreenMode)
{
    if (inFullscreenMode != 0 && inFullscreenMode != 1) {
        LOG_FATAL("Invalid fullscreenMode value: %u", inFullscreenMode);
    }
    fullscreenMode = inFullscreenMode;
}

ScreenRect UserConfig::getWindowSize()
{
    return {0, 0, windowSizeWidth, windowSizeHeight};
}

void UserConfig::setWindowSize(ScreenRect inWindowSize)
{
    // Default to the given window size.
    windowSizeWidth = inWindowSize.width;
    windowSizeHeight = inWindowSize.height;

    // If fullscreen is selected, use the desktop size instead.
    // Note: SDL_GetDesktopDisplayMode() requires the SDL video subsystem
    //       to be initialized, so we have to check for that first.
    if ((fullscreenMode != 0) && SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_DisplayMode displayMode;
        SDL_GetDesktopDisplayMode(0, &displayMode);
        windowSizeWidth = static_cast<float>(displayMode.w);
        windowSizeHeight = static_cast<float>(displayMode.h);
    }
}

unsigned int UserConfig::getFramesPerSecond()
{
    return framesPerSecond;
}

double UserConfig::getFrameTimestepS()
{
    return frameTimestepS;
}

void UserConfig::setFramesPerSecond(unsigned int inFramesPerSecond)
{
    framesPerSecond = inFramesPerSecond;
    frameTimestepS = (1.0 / static_cast<double>(framesPerSecond));
}

ServerAddress UserConfig::getServerAddress()
{
    return {serverIP, serverPort};
}

void UserConfig::setServerAddress(const ServerAddress& inServerAddress)
{
    serverIP = inServerAddress.IP;
    serverPort = inServerAddress.port;
}

void UserConfig::init(nlohmann::json& json)
{
    // Fullscreen mode.
    setFullscreenMode(json.at("fullscreenMode"));

    // Window size.
    setWindowSize(
        {0, 0, json.at("windowSizeWidth"), json.at("windowSizeHeight")});

    // Server address.
    setServerAddress({json.at("serverIP"), json.at("serverPort")});

    // Framerate.
    setFramesPerSecond(json.at("framesPerSecond"));
}

} // End namespace Client
} // End namespace AM
