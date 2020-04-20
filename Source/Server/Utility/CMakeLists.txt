target_sources(Server
	PRIVATE
		Private/
)

target_include_directories(Server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)
