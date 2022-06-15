#pragma once

#include <SDL_stdinc.h>
#include <atomic>
#include <cstdlib>
#include <string>

/**
 * Use these macros instead of calling the functions directly.
 */
#define LOG_INFO(...)                                                          \
    do {                                                                       \
        AM::Log::info(__VA_ARGS__);                                            \
    } while (false)

#ifndef NDEBUG
#define LOG_ERROR(...)                                                         \
    do {                                                                       \
        AM::Log::error(__FILE__, __LINE__, __VA_ARGS__);                       \
        std::abort();                                                          \
    } while (false)
#else
#define LOG_ERROR(...)                                                         \
    do {                                                                       \
        AM::Log::error(__FILE__, __LINE__, __VA_ARGS__);                       \
    } while (false)
#endif

#define LOG_FATAL(...)                                                         \
    do {                                                                       \
        AM::Log::error(__FILE__, __LINE__, __VA_ARGS__);                       \
        std::abort();                                                          \
    } while (false)

namespace AM
{
/**
 * Facilitates logging info and errors to stdout or a log file.
 *
 * Our logging system has 3 levels:
 *   LOG_INFO: Print the given string in release and debug.
 *   LOG_ERROR: Print file name, line number, and the given string. In debug
 *             this will also std::abort().
 *   LOG_FATAL: Print file name, line number, and the given string. In debug
 *              and release this will also std::abort().
 *
 * Use LOG_INFO for general printing, LOG_ERROR for recoverable errors (make
 * sure you write appropriate recovery logic), and LOG_FATAL for unrecoverable
 * errors.
 * Generally, we'll start error cases as LOG_FATAL, then switch them to
 * LOG_ERROR if there's some expected failure that we can't yet fix.
 */
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
     * called.), then flushes the buffer.
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
