#include "Log.h"
#include <cstdio>
#include <cstdarg>
#include <string>

namespace AM
{
const std::atomic<Uint32>* Log::currentTickPtr = nullptr;
FILE* logFilePtr = nullptr;

void Log::registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Log::info(const char* expression, ...)
{
    // If the app hasn't registered a tick count, default to 0.
    Uint32 currentTick = 0;
    if (currentTickPtr != nullptr) {
        currentTick = *currentTickPtr;
    }

    std::va_list arg;
    va_start(arg, expression);

    // Write to stdout.
    std::printf("Tick %u: ", currentTick);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    // Write to file.
    if (logFilePtr != nullptr) {
        std::fprintf(logFilePtr, "Tick %u: ", currentTick);
        std::vfprintf(logFilePtr, expression, arg);
        std::fprintf(logFilePtr, "\n");
        std::fflush(logFilePtr);
    }

    va_end(arg);
}

void Log::error(const char* fileName, int line, const char* expression, ...)
{
    // If the app hasn't registered a tick count, default to 0.
    Uint32 currentTick = 0;
    if (currentTickPtr != nullptr) {
        currentTick = *currentTickPtr;
    }

    std::va_list arg;
    va_start(arg, expression);

    // Write to stdout.
    std::printf("Error at file: %s, line: %d, during tick: %u\n", fileName,
                line, currentTick);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    // Write to file.
    if (logFilePtr != nullptr) {
        std::fprintf(logFilePtr, "Error at file: %s, line: %d, during tick: %u\n", fileName,
                    line, currentTick);
        std::vfprintf(logFilePtr, expression, arg);
        std::fprintf(logFilePtr, "\n");
        std::fflush(logFilePtr);
    }

    va_end(arg);
}

void Log::enableFileLogging(const std::string& fileName)
{
    // Open the log file.
    logFilePtr = fopen(fileName.c_str(), "w");
    if (logFilePtr == nullptr) {
        std::printf("Failed to open log file for writing.\n");
    }
}

} // namespace AM
