target_sources(Server
	PRIVATE
		Private/main.cpp
)

target_include_directories(Server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
