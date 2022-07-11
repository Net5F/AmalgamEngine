#pragma once

#include "Log.h"

/**
 * Assert macro. Use this in place of assert().
 */
#ifdef NDEBUG
#define AM_ASSERT(condition, ...)                                              \
    do {                                                                       \
    } while (false)
#else
#define AM_ASSERT(condition, ...)                                              \
    do {                                                                       \
        if (!(condition)) {                                                    \
            LOG_ERROR(__VA_ARGS__);                                            \
        }                                                                      \
    } while (false)
#endif
