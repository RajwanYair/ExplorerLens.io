# src/Tools.CLI/CMakeLists.txt — lens.exe CLI Tool Target
# Copyright (c) 2026 ExplorerLens Project
#
# Builds lens.exe linking ExplorerLensEngine.lib.
# Requires C++20 and WIN32 (DXGI for doctor, filesystem, COM for register).
#
# Split architecture: LensCLILib (static library) + LensCLI (thin executable).
# EngineTests links LensCLILib for unit-testing CLI layer without duplicating sources.

cmake_minimum_required(VERSION 3.20)

# --------------------------------------------------------------------------
# LensCLILib — static library containing all CLI logic (no main.cpp)
# --------------------------------------------------------------------------
add_library(LensCLILib STATIC
    CommandRouter.cpp
    GenerateCommand.cpp
    InfoCommand.cpp
    CacheCommand.cpp
    RegisterCommand.cpp
    BenchmarkCommand.cpp
    DoctorCommand.cpp
)

# --------------------------------------------------------------------------
# LensCLI — thin executable: just main.cpp + LensCLILib
# --------------------------------------------------------------------------
add_executable(LensCLI
    main.cpp
)

# --------------------------------------------------------------------------
# LensCLILib — compile settings (library)
# --------------------------------------------------------------------------
set_target_properties(LensCLILib PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(LensCLILib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Engine
)

if(MSVC)
    target_compile_options(LensCLILib PRIVATE
        /W4
        /permissive-
        /utf-8
    )
    target_compile_definitions(LensCLILib PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
        _WIN32_WINNT=0x0A00
    )
endif()

target_link_libraries(LensCLILib
    PUBLIC
        ExplorerLensEngine
    PRIVATE
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

# --------------------------------------------------------------------------
# LensCLI — executable settings
# --------------------------------------------------------------------------
set_target_properties(LensCLI PROPERTIES
    OUTPUT_NAME "lens"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(LensCLI PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(LensCLI PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0A00
    )
endif()

target_link_libraries(LensCLI
    PRIVATE
        LensCLILib
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
