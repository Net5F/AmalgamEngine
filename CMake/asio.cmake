cmake_minimum_required(VERSION 3.5)

# Add our header-only library target.
add_library(asio INTERFACE)

target_include_directories(asio
    INTERFACE
        ${PROJECT_SOURCE_DIR}/Libraries/asio/include/
)

target_sources(asio INTERFACE
    ${PROJECT_SOURCE_DIR}/Libraries/asio/include/asio.hpp
)

# Configure standalone Asio and propagate its platform dependencies.
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

find_package(Threads REQUIRED)
target_link_libraries(asio INTERFACE Threads::Threads)

if (WIN32)
    # Fixes a warning. 0x0601 == Windows 7.
    target_compile_definitions(asio INTERFACE
        _WIN32_WINNT=0x0601
        WINVER=0x0601
    )

    target_link_libraries(asio INTERFACE ws2_32 mswsock)
endif()
