target_sources(Shared
	PUBLIC
		Public/ScreenPoint.h
		Public/ScreenRect.h
)

target_include_directories(Shared
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/Public
)
