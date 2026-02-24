cmake_minimum_required(VERSION 3.20)

add_library(ExplorerLensModernRuntime STATIC
    Worker/ipc_transport.cpp
    Worker/worker_process.cpp
    ShellHost/shellhost_client.cpp
    Service/LocalApiService.cpp
)

target_include_directories(ExplorerLensModernRuntime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(WIN32)
    target_compile_definitions(ExplorerLensModernRuntime PRIVATE
        _WINDOWS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(ExplorerLensModernRuntime
        PRIVATE
            advapi32
            user32
            gdi32
            kernel32
    )
endif()

if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(ExplorerLensModernRuntime PRIVATE -Wall -Wextra)
    else()
        target_compile_options(ExplorerLensModernRuntime PRIVATE /W4 /permissive-)
    endif()
endif()

