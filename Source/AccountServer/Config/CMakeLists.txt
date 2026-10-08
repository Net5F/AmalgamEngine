target_sources(AccountServer
    PRIVATE
        Private/UserConfig.cpp
        Private/UserConfigInitializer.cpp
    PUBLIC
        Public/Config.h
        Public/UserConfig.h
        Public/UserConfigInitializer.h
)

target_include_directories(AccountServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
