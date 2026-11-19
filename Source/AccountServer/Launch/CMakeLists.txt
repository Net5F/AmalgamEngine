target_sources(AccountServer
    PRIVATE
        Private/main.cpp
        Private/CommandLineTool.cpp
        Private/Application.cpp
    PUBLIC
        Public/Application.h
        Public/CommandLineTool.h
)

target_include_directories(AccountServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
