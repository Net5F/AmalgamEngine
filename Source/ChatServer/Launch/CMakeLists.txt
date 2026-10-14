target_sources(ChatServer
    PRIVATE
        Private/main.cpp
        Private/Application.cpp
    PUBLIC
        Public/Application.h
)

target_include_directories(ChatServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
