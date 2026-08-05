cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring libsodium")

set(LIBSODIUM_SOURCE_DIR
    ${PROJECT_SOURCE_DIR}/Libraries/libsodium/src/libsodium)
set(LIBSODIUM_GENERATED_INCLUDE_DIR
    ${PROJECT_BINARY_DIR}/Libraries/libsodium/include)

# The repository keeps the generated version header with its MSVC build files.
# Copy it into the build tree so the submodule's source tree remains untouched.
file(MAKE_DIRECTORY ${LIBSODIUM_GENERATED_INCLUDE_DIR}/sodium)
configure_file(
    ${PROJECT_SOURCE_DIR}/Libraries/libsodium/builds/msvc/version.h
    ${LIBSODIUM_GENERATED_INCLUDE_DIR}/sodium/version.h
    COPYONLY
)

# Libsodium doesn't provide a CMake project. Its own MSVC and Zig builds compile
# every C source under src/libsodium, so do the same here.
file(GLOB_RECURSE LIBSODIUM_SOURCES CONFIGURE_DEPENDS
    ${LIBSODIUM_SOURCE_DIR}/*.c
)

# Add our static library target.
add_library(libsodium STATIC
    ${LIBSODIUM_SOURCES}
    ${LIBSODIUM_SOURCE_DIR}/include/sodium.h
    ${LIBSODIUM_GENERATED_INCLUDE_DIR}/sodium/version.h
)

target_include_directories(libsodium
    PRIVATE
        ${LIBSODIUM_GENERATED_INCLUDE_DIR}/sodium
    PUBLIC
        ${LIBSODIUM_GENERATED_INCLUDE_DIR}
        ${LIBSODIUM_SOURCE_DIR}/include
        ${LIBSODIUM_SOURCE_DIR}/include/sodium
)

# Consumers of the static library must not import its public symbols from a DLL.
target_compile_definitions(libsodium
    PRIVATE
        CONFIGURED=1
        DEV_MODE=1
    PUBLIC
        SODIUM_STATIC
)

include(TestBigEndian)
test_big_endian(LIBSODIUM_BIG_ENDIAN)
if (LIBSODIUM_BIG_ENDIAN)
    target_compile_definitions(libsodium PRIVATE NATIVE_BIG_ENDIAN=1)
else()
    target_compile_definitions(libsodium PRIVATE NATIVE_LITTLE_ENDIAN=1)
endif()

if (MSVC)
    # Match the settings in libsodium's maintained Visual Studio projects.
    target_compile_definitions(libsodium PRIVATE
        inline=__inline
        _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(libsodium PRIVATE /wd4146 /wd4244)
    target_link_libraries(libsodium PRIVATE advapi32)
else()
    target_compile_definitions(libsodium PRIVATE
        _GNU_SOURCE=1
        HAVE_ATOMIC_OPS=1
        HAVE_C11_MEMORY_FENCES=1
        HAVE_GCC_MEMORY_FENCES=1
        HAVE_INLINE_ASM=1
        HAVE_INTTYPES_H=1
        HAVE_STDINT_H=1
    )
    target_compile_options(libsodium PRIVATE
        -fno-strict-aliasing
        -fno-strict-overflow
        -fwrapv
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(libsodium PRIVATE
            HAVE_CATCHABLE_ABRT=1
            HAVE_CATCHABLE_SEGV=1
            HAVE_CLOCK_GETTIME=1
            HAVE_GETPID=1
            HAVE_MADVISE=1
            HAVE_MLOCK=1
            HAVE_MMAP=1
            HAVE_MPROTECT=1
            HAVE_NANOSLEEP=1
            HAVE_POSIX_MEMALIGN=1
            HAVE_PTHREAD=1
            HAVE_PTHREAD_PRIO_INHERIT=1
            HAVE_RAISE=1
            HAVE_SYSCONF=1
            HAVE_SYS_AUXV_H=1
            HAVE_SYS_MMAN_H=1
            HAVE_SYS_PARAM_H=1
            HAVE_SYS_RANDOM_H=1
            HAVE_WEAK_SYMBOLS=1
            TLS=_Thread_local
        )
    elseif (APPLE)
        target_compile_definitions(libsodium PRIVATE
            HAVE_ARC4RANDOM=1
            HAVE_ARC4RANDOM_BUF=1
            HAVE_CATCHABLE_ABRT=1
            HAVE_CATCHABLE_SEGV=1
            HAVE_CLOCK_GETTIME=1
            HAVE_GETENTROPY=1
            HAVE_GETPID=1
            HAVE_MADVISE=1
            HAVE_MEMSET_S=1
            HAVE_MLOCK=1
            HAVE_MMAP=1
            HAVE_MPROTECT=1
            HAVE_NANOSLEEP=1
            HAVE_POSIX_MEMALIGN=1
            HAVE_PTHREAD=1
            HAVE_PTHREAD_PRIO_INHERIT=1
            HAVE_RAISE=1
            HAVE_SYSCONF=1
            HAVE_SYS_MMAN_H=1
            HAVE_SYS_PARAM_H=1
            HAVE_SYS_RANDOM_H=1
            HAVE_WEAK_SYMBOLS=1
            TLS=_Thread_local
        )
    endif()

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(libsodium PRIVATE Threads::Threads)
    endif()
endif()

set_target_properties(libsodium PROPERTIES
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)
