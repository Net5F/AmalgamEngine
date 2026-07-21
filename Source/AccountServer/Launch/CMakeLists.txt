target_sources(AccountServer
    PRIVATE
        Private/main.cpp
        Private/Application.cpp
    PUBLIC
        Public/Application.h
)

target_include_directories(AccountServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
