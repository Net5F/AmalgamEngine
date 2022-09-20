# Support for building with AddressSanitizer (ASan) -
# https://code.google.com/p/address-sanitizer/
#
# Note: Including this file enables ASan for all targets in the current 
#       directory and all sub-directories (that are added after including 
#       this file).

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CMakePushCheckState)

CMAKE_PUSH_CHECK_STATE(RESET)
SET(CMAKE_REQUIRED_FLAGS "-fsanitize=address") # Also needs to be a link flag for test to pass
CHECK_C_COMPILER_FLAG("-fsanitize=address" HAVE_FLAG_SANITIZE_ADDRESS_C)
CHECK_CXX_COMPILER_FLAG("-fsanitize=address" HAVE_FLAG_SANITIZE_ADDRESS_CXX)
CMAKE_POP_CHECK_STATE()

IF(HAVE_FLAG_SANITIZE_ADDRESS_C AND HAVE_FLAG_SANITIZE_ADDRESS_CXX)
    SET(ADDRESS_SANITIZER_FLAG "-fsanitize=address")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADDRESS_SANITIZER_FLAG}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDRESS_SANITIZER_FLAG}")
    SET(CMAKE_CGO_LDFLAGS "${CMAKE_CGO_LDFLAGS} ${ADDRESS_SANITIZER_FLAG}")

    ADD_DEFINITIONS(-DADDRESS_SANITIZER)

    MESSAGE(STATUS "AM: AddressSanitizer enabled.")
ELSE()
    MESSAGE(FATAL_ERROR "AM_ADDRESSSANITIZER enabled but compiler doesn't support AddressSanitizer - cannot continue.")
ENDIF()
