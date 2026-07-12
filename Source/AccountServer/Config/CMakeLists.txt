target_sources(AccountServer
    PUBLIC
        Public/Config.h
)

target_include_directories(AccountServer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
