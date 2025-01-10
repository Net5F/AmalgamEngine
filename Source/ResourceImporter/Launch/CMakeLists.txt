target_sources(ResourceImporter
    PRIVATE
        Private/main.cpp
        Private/Application.cpp
    PUBLIC
        Public/Application.h
)

target_include_directories(ResourceImporter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
