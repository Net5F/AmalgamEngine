target_sources(Client
	PRIVATE
		Private/TransformationHelpers.cpp
	PUBLIC
	    Public/EventHandler.h
		Public/TransformationHelpers.h
)

target_include_directories(Client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
