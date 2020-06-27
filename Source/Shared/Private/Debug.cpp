#include "Debug.h"
#include <cstdio>
#include <cstdarg>
#include <atomic>

namespace AM
{

const std::atomic<Uint32>* Debug::currentTickPtr = nullptr;

void Debug::registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Debug::info(const char* expression, ...)
{
#if defined(ENABLE_DEBUG_INFO)
    // If the app hasn't registered a tick count, default to 0.
    Uint32 currentTick = 0;
    if (currentTickPtr != nullptr) {
        currentTick = *currentTickPtr;
    }

    std::va_list arg;
    va_start(arg, expression);

    std::printf(" %u: ", currentTick);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    va_end(arg);
#endif // ENABLE_DEBUG_INFO
}

void Debug::error(const char* fileName, int line, const char* expression, ...)
{
#if defined(ENABLE_DEBUG_INFO)
    // If the app hasn't registered a tick count, default to 0.
    Uint32 currentTick = 0;
    if (currentTickPtr != nullptr) {
        currentTick = *currentTickPtr;
    }

    std::va_list arg;
    va_start(arg, expression);

    std::printf(" Error at file: %s, line: %d, during tick: %u\n", fileName, line,
        currentTick);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    va_end(arg);
#endif // ENABLE_DEBUG_INFO
}

}
