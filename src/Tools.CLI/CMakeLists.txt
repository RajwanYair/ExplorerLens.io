# src/Tools.CLI/CMakeLists.txt — lens.exe CLI Tool Target
# Copyright (c) 2026 ExplorerLens Project
#
# Sprint 24 (v15.4.0 "Zenith-U"): Builds lens.exe linking ExplorerLensEngine.lib.
# Requires C++20 and WIN32 (DXGI for doctor, filesystem, COM for register).

cmake_minimum_required(VERSION 3.20)

add_executable(LensCLI
    main.cpp
    CommandRouter.cpp
    GenerateCommand.cpp
    InfoCommand.cpp
    CacheCommand.cpp
    RegisterCommand.cpp
    BenchmarkCommand.cpp
    DoctorCommand.cpp
)

set_target_properties(LensCLI PROPERTIES
    OUTPUT_NAME "lens"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(LensCLI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Engine
)

if(MSVC)
    target_compile_options(LensCLI PRIVATE
        /W4
        /permissive-
        /utf-8
        /wd4100   # unreferenced formal parameter (DoctorCommand output helpers)
    )
    target_compile_definitions(LensCLI PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
        _WIN32_WINNT=0x0A00
    )
endif()

target_link_libraries(LensCLI
    PRIVATE
        ExplorerLensEngine
        ExplorerLensModernRuntime
        ole32
        oleaut32
        shlwapi
        dxgi
        d3d11
        advapi32
        user32
        kernel32
)

# Install alongside the engine artifacts
install(TARGETS LensCLI
    RUNTIME DESTINATION bin
    COMPONENT CLI
)

# Install autocomplete script
install(FILES ${CMAKE_SOURCE_DIR}/scripts/lens-autocomplete.ps1
    DESTINATION scripts
    COMPONENT CLI
)
