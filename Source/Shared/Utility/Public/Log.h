#pragma once

#include <SDL_stdinc.h>
#include <atomic>

/**
 * Use these macros instead of calling the functions directly.
 *
 * Debug::info could be called directly, but error must be called from the macro
 * to get the proper file and line number, so we choose to use both macros for
 * consistency.
 */
#define LOG_INFO(...)                                                         \
    {                                                                          \
        Log::info(__VA_ARGS__);                                              \
    }

#define LOG_ERROR(...)                                                        \
    {                                                                          \
        Log::error(__FILE__, __LINE__, __VA_ARGS__);                         \
        abort();                                                               \
    }

namespace AM
{
class Log
{
public:
    static void
        registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Prints the given info to stdout and flushes the buffer.
     * Note: The implementation is hidden behind the ENABLE_DEBUG_INFO flag.
     *       The cmake logic should be such that this only happens in debug
     *       config.
     */
    static void info(const char* expression, ...);

    /**
     * Prints the given info to stdout, flushes the buffer,
     * then calls abort() to crash the program.
     * Note: The implementation is hidden behind the ENABLE_DEBUG_INFO flag.
     *       The cmake logic should be such that this only happens in debug
     *       config.
     */
    static void error(const char* fileName, int line, const char* expression,
                      ...);

private:
    static const std::atomic<Uint32>* currentTickPtr;
};

} /* End namespace AM */
