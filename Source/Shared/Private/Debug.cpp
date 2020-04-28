#include "Debug.h"
#include <cstdio>
#include <cstdarg>

namespace AM
{

Uint32* Debug::currentTickPtr = nullptr;

void Debug::registerCurrentTickPtr(Uint32* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Debug::info(const char* expression, ...)
{
#if defined(ENABLE_DEBUG_INFO)
    if (currentTickPtr == nullptr) {
        std::printf("WARNING: Tried to print debug info without registering tick ptr.");
        return;
    }

    std::va_list arg;
    va_start(arg, expression);

    std::printf("%u: ", *currentTickPtr);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    va_end(arg);
#endif // ENABLE_DEBUG_INFO
}

void Debug::error(const char* fileName, int line, const char* expression, ...)
{
#if defined(ENABLE_DEBUG_INFO)
    if (currentTickPtr == nullptr) {
        std::printf("WARNING: Tried to cause debug error without registering tick ptr.");
        return;
    }

    std::va_list arg;
    va_start(arg, expression);

    std::printf("Error at file: %s, line: %d, during tick: %u\n", fileName, line,
        *currentTickPtr);
    std::vprintf(expression, arg);
    std::printf("\n");
    std::fflush(stdout);

    va_end(arg);
#endif // ENABLE_DEBUG_INFO
}

}
