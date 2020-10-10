#include "Log.h"
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace AM
{
// Init the member vars.
std::shared_ptr<spdlog::logger> Log::engineLogger;
const std::atomic<Uint32>* Log::currentTickPtr = nullptr;
Uint32 Log::lastLoggedTick = 0;

void Log::registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Log::init()
{
    std::vector<spdlog::sink_ptr> logSinks;
    logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    logSinks.emplace_back(
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("Amalgam.log", true));

    logSinks[0]->set_pattern("%v");
    logSinks[1]->set_pattern("%v");

    engineLogger = std::make_shared<spdlog::logger>("Engine", logSinks.begin(),
        logSinks.end());
    spdlog::register_logger(engineLogger);
    engineLogger->set_level(spdlog::level::trace);
    engineLogger->flush_on(spdlog::level::trace);
}

} // namespace AM
