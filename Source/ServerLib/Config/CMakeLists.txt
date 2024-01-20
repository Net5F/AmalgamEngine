target_sources(ServerLib
    PRIVATE
        Private/UserConfig.cpp
        Private/UserConfigInitializer.cpp
    PUBLIC
        Public/UserConfig.h
        Public/UserConfigInitializer.h
)

target_include_directories(ServerLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
