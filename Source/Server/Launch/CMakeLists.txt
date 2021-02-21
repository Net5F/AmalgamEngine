target_sources(Server
    PRIVATE
		Private/main.cpp
		Private/Application.cpp
    PUBLIC
		Public/Application.h
)

target_include_directories(Server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
