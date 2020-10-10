#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include <SDL_stdinc.h>
#include <atomic>
#include <utility>

// Logging macros, use these instead of calling the Log functions directly.
#define LOG_INFO(...)                                                     \
{                                                                          \
    ::AM::Log::info(__VA_ARGS__);                                          \
}

#define LOG_ERROR(...)                                                    \
{                                                                          \
    ::AM::Log::error(__FILE__, __LINE__, __VA_ARGS__);                     \
    abort();                                                               \
}

namespace AM
{
class Log
{
public:
    /**
     * Used to register the sim tick. Current tick number will be added to the log.
     */
    static void
        registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Initializes the logger, setting up the patterns and sinks.
     */
    static void init();

    /**
     */
    template<typename FormatString, typename... Args>
    static void info(const FormatString& fmt, const Args& ...args)
    {
        // Get the latest sim tick.
        Uint32 currentTick = 0;
        if ((currentTickPtr != nullptr)) {
            currentTick = *currentTickPtr;
        }

        // If the tick has changed since we last logged, log it.
        if (currentTick > lastLoggedTick) {
            engineLogger->info("Tick {} ----------", currentTick);
        }

        // Log the given info.
        engineLogger->info(fmt, args...);
    }

    /**
     */
    template<typename FormatString, typename... Args>
    static void error(const char* fileName, int line, const FormatString& fmt,
                          const Args& ...args)
    {
        // Get the latest sim tick.
        Uint32 currentTick = 0;
        if ((currentTickPtr != nullptr)) {
            currentTick = *currentTickPtr;
        }

        // Log the file name, line number, and tick number.
        engineLogger->error("Error at file: {}, line: {}, during tick: {}",
                fileName, line, currentTick);

        // Log the given info.
        engineLogger->error(fmt, args...);
    }

private:
    static std::shared_ptr<spdlog::logger> engineLogger;

    /**
     * A pointer to the sim tick. Registered through registerCurrentTickPtr and
     * used as a timestamp in the log.
     */
    static const std::atomic<Uint32>* currentTickPtr;

    /** Holds the last tick number that we logged. */
    static Uint32 lastLoggedTick;
};

} /* End namespace AM */
