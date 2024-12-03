target_sources(ResourceImporter
    PRIVATE
        Private/Renderer.cpp
    PUBLIC
        Public/Renderer.h
)

target_include_directories(ResourceImporter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
