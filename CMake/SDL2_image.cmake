cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring SDL_image")

# Add our static library target
add_library(SDL2_image-static STATIC
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_png.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_bmp.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_gif.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_jpg.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_lbm.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_pcx.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_pnm.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_svg.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_tga.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_tif.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_webp.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_WIC.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_xcf.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_xpm.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_xv.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/IMG_xxx.c
)
set_target_properties(SDL2_image-static PROPERTIES OUTPUT_NAME "SDL2_image")

target_compile_definitions(SDL2_image-static PRIVATE
		-DLOAD_BMP -DLOAD_GIF -DLOAD_LBM -DLOAD_PCX -DLOAD_PNM
		-DLOAD_TGA -DLOAD_XCF -DLOAD_XPM -DLOAD_XV -DLOAD_XPM)

if (SUPPORT_JPG)
	target_compile_definitions(SDL2_image-static PRIVATE -DLOAD_JPG)
	add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/external/jpeg-9c"
	                 "${PROJECT_BINARY_DIR}/Libraries/SDL_image/external/jpeg-9c")
	target_link_libraries(SDL2_image-static PRIVATE jpeg)
endif()

if (SUPPORT_PNG)
	# missing libpng.vers
	set(HAVE_LD_VERSION_SCRIPT OFF CACHE BOOL "" FORCE)
	target_compile_definitions(SDL2_image-static PRIVATE -DLOAD_PNG)

    set(PNG_SHARED OFF CACHE BOOL "libpng build shared lib.")
    set(PNG_STATIC ON CACHE BOOL "libpng build static lib.")
	add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/external/libpng-1.6.37"
	                 "${PROJECT_BINARY_DIR}/Libraries/SDL_image/external/libpng-1.6.37")
	include_directories(${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/external/libpng-1.6.37)
	target_link_libraries(SDL2_image-static PRIVATE png_static)
endif()

if (SUPPORT_WEBP)
	target_compile_definitions(SDL2_image-static PRIVATE -DLOAD_WEBP)
	# missing cpufeatures
	add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/SDL_image/external/libwebp-1.0.3"
	                 "${PROJECT_BINARY_DIR}/Libraries/SDL_image/external/libwebp-1.0.3")
	target_link_libraries(SDL2_image-static PRIVATE webp)
endif()

target_include_directories(SDL2_image-static
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../Libraries/SDL_image
)

target_link_libraries(SDL2_image-static PRIVATE SDL2-static)
