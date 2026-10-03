target_sources(SharedLib
    PRIVATE
        Private/AccountHelpers.cpp
    PUBLIC
        Public/AccountDefs.h
        Public/AccountHelpers.h
)

target_include_directories(SharedLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
