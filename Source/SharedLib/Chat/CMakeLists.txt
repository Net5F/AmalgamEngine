target_sources(SharedLib
    PRIVATE
        Private/ChatHelpers.cpp
    PUBLIC
        Public/ChatDefs.h
        Public/ChatHelpers.h
)

target_include_directories(SharedLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
