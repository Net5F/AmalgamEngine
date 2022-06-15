target_sources(ServerLib
    PUBLIC
        Public/Config.h
)

target_include_directories(ServerLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
