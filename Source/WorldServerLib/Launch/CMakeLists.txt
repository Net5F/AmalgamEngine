target_sources(WorldServerLib
    PRIVATE
		Private/Application.cpp
    PUBLIC
		Public/Application.h
)

target_include_directories(WorldServerLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
