#pragma once

#include "Log.h"

/**
 * Assert macro. Use this in place of assert().
 */
#ifndef NDEBUG
#define AM_ASSERT(condition, ...)                                              \
    do {                                                                       \
        if (!(condition)) {                                                    \
            LOG_ERROR(__VA_ARGS__);                                            \
        }                                                                      \
    } while (false)
#else
#define AM_ASSERT(condition, ...)                                              \
    do {                                                                       \
    } while (false)
#endif
