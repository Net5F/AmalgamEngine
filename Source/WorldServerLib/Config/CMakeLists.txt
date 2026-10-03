target_sources(WorldServerLib
    PRIVATE
        Private/UserConfig.cpp
        Private/UserConfigInitializer.cpp
    PUBLIC
        Public/UserConfig.h
        Public/UserConfigInitializer.h
)

target_include_directories(WorldServerLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
