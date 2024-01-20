target_sources(ClientLib
    PRIVATE
        Private/UserConfig.cpp
        Private/UserConfigInitializer.cpp
    PUBLIC
        Public/UserConfig.h
        Public/UserConfigInitializer.h
        Public/UserConfigStructs.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
