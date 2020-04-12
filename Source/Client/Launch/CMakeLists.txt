target_sources(Client
	PRIVATE
		Private/Launch.cpp
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
