cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Shared")

# Add our static library target
add_library(Shared STATIC
	PRIVATE
		Private/Acceptor.cpp
		Private/Peer.cpp
		Private/Timer.cpp
		Private/MovementHelpers.cpp
		Private/NetworkHelpers.cpp
		Private/MessageUtil.cpp
		Private/Debug.cpp
	PUBLIC
		Public/Acceptor.h
		Public/Peer.h
		Public/Timer.h
		Public/MovementHelpers.h
		Public/NetworkHelpers.h
		Public/GameDefs.h
		Public/NetworkDefs.h
		Public/MessageUtil.h
		Public/Debug.h

		Public/InputComponent.h
		Public/MovementComponent.h
		Public/PositionComponent.h
		Public/SpriteComponent.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

#target_precompile_headers(Shared PRIVATE Private/pch.h)

target_link_libraries(Shared
    PUBLIC
        ${SDL2_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        SDL2pp
        flatbuffers
)

# Compile with C++17
target_compile_features(Shared PRIVATE cxx_std_17)
set_target_properties(Shared PROPERTIES CXX_EXTENSIONS OFF)

# If debug, enable debug printing.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(Shared PUBLIC -DENABLE_DEBUG_INFO)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
