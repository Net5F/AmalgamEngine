#pragma once

#include "Remotery.h"
#include "Ignore.h"

/**
 * Begins recording a CPU time sample of name sampleName.
 * Runs until END_CPU_SAMPLE is called.
 * @param sampleName  The name of the sample. Plain text, no quotes.
 */
#define BEGIN_CPU_SAMPLE(sampleName)                                           \
    {                                                                          \
        rmt_BeginCPUSample(sampleName, 0);                                     \
    }

/**
 * Ends a BEGIN_CPU_SAMPLE that was called in the same scope.
 */
#define END_CPU_SAMPLE()                                                       \
    {                                                                          \
        rmt_EndCPUSample();                                                    \
    }

/**
 * Begins recording a CPU time sample of name sampleName.
 * Runs until the end of the enclosing scope.
 * @param sampleName  The name of the sample. Plain text, no quotes.
 */
#define SCOPED_CPU_SAMPLE(sampleName)                                          \
    {                                                                          \
        rmt_ScopedCPUSample(sampleName, 0);                                    \
    }

namespace AM
{
/**
 * Wraps our chosen profiling library to facilitate easy refactoring.
 *
 * Ideally all of the functionality would be exposed through functions in this
 * class, but we choose to use macros, in case the library is using __FILE__
 * and __LINE__ calls.
 */
class Profiler
{
public:
    /**
     * Initializes the profiler.
     */
    static void init()
    {
#if defined(PROFILING_ENABLED)
        Remotery* rmt;
        rmt_CreateGlobalInstance(&rmt);
#endif
    }
};

} // namespace AM
