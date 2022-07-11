target_sources(ClientLib
    PUBLIC
        Public/Config.h
)

target_include_directories(ClientLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
