target_sources(Server
    PUBLIC
        Public/Config.h
)

target_include_directories(Server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
