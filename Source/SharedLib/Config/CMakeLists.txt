target_sources(SharedLib
    PUBLIC
        Public/SharedConfig.h
)

target_include_directories(SharedLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
