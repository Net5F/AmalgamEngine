cmake_minimum_required(VERSION 3.5)

message(STATUS "Building Shared")

# Add our static library target
add_library(Shared STATIC
	PRIVATE
		Private/Acceptor.cpp
		Private/Peer.cpp
		Private/Timer.cpp
		Private/MessageUtil.cpp
	PUBLIC
		Public/Acceptor.h
		Public/Peer.h
		Public/Timer.h

		Public/InputComponent.h
		Public/MovementComponent.h
		Public/PositionComponent.h
		Public/SpriteComponent.h

		Public/SharedDefs.h
		Public/MessageUtil.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_link_libraries(Shared
    PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        SDL2pp
        flatbuffers
)

# Compile with C++17
target_compile_features(Shared PRIVATE cxx_std_17)
set_target_properties(Shared PROPERTIES CXX_EXTENSIONS OFF)

