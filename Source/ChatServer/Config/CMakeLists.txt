target_sources(ChatServer
    PRIVATE
        Private/UserConfig.cpp
        Private/UserConfigInitializer.cpp
    PUBLIC
        Public/Config.h
        Public/UserConfig.h
        Public/UserConfigInitializer.h
)

target_include_directories(ChatServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
