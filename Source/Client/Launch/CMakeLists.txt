target_sources(Client
	PRIVATE
		Private/main.cpp
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
