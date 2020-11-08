#pragma once

#include <SDL_stdinc.h>
#include <atomic>
#include <cstdlib>
#include <string>

/**
 * Use these macros instead of calling the functions directly.
 */
#define LOG_INFO(...)                                                          \
    {                                                                          \
        Log::info(__VA_ARGS__);                                                \
    }

#define LOG_ERROR(...)                                                         \
    {                                                                          \
        Log::error(__FILE__, __LINE__, __VA_ARGS__);                           \
        std::abort();                                                          \
    }

namespace AM
{
class Log
{
public:
    static void
        registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr);

    /**
     * Prints the given info to stdout (and a file, if enableFileLogging() was
     * called.), then flushes the buffer.
     */
    static void info(const char* expression, ...);

    /**
     * Prints the given info to stdout (and a file, if enableFileLogging() was
     * called.), then flushes the buffer and calls abort().
     */
    static void error(const char* fileName, int line, const char* expression,
                      ...);

    /**
     * Opens a file with the given file name and enables file logging.
     */
    static void enableFileLogging(const std::string& fileName);

private:
    /**
     * Should be passed the sim's tick through registerCurrentTickPtr.
     * Used to print a timestamp that's more relevant than wall time.
     */
    static const std::atomic<Uint32>* currentTickPtr;

    /** Used to safely test if currentTickPtr is ready to use. */
    static std::atomic<bool> tickPtrIsRegistered;
};

} /* End namespace AM */
