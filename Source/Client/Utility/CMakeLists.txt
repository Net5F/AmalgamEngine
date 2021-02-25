target_sources(Client
	PUBLIC
	    Public/EventHandler.h
)

target_include_directories(Client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
