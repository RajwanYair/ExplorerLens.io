#==============================================================================
# LENSManager — GUI Configuration Utility (WTL-based)
#
# This CMakeLists.txt mirrors the MSBuild LENSManager.vcxproj configuration
# for unified CMake build system integration.
#==============================================================================

cmake_minimum_required(VERSION 3.20)
project(LENSManager LANGUAGES CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "LENSManager is x64-only.")
endif()

#==============================================================================
# Source Files
#==============================================================================

set(SOURCES
    About.cpp
    LENSManager.cpp
    MainDlg.cpp
    stdafx.cpp
)

set(HEADERS
    # Core
    About.h
    LENSManager.h
    MainDlg.h
    RegManager.h
    resource.h
    stdafx.h
    tools.h

    # UI (v15.0.0 — Dark Mode / Mica / Acrylic)
    ChangeSummaryDlg.h
    DarkModeController.h
    DarkModeHelper.h
    SystemTrayIcon.h

    # Diagnostics & Health
    DecoderHealthCheck.h
    ExportDiagnostics.h
    PerformanceDashboard.h

    # Format Management
    FormatGroupHelper.h
    FormatStatusProvider.h

    # Settings
    SettingsPortability.h
)

set(RESOURCES
    LENSManager.rc
)

#==============================================================================
# Executable Target
#==============================================================================

add_executable(LENSManager WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

#==============================================================================
# Precompiled Header (stdafx.h)
#==============================================================================

target_precompile_headers(LENSManager PRIVATE stdafx.h)

#==============================================================================
# Include Directories
#==============================================================================

# WTL include path (NuGet package or external)
set(WTL_DIR ${CMAKE_SOURCE_DIR}/packages/wtl.10.0.10320/lib/native/include)
if(NOT EXISTS ${WTL_DIR})
    set(WTL_DIR ${CMAKE_SOURCE_DIR}/external/ui-libs/wtl)
endif()

target_include_directories(LENSManager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WTL_DIR}
)

#==============================================================================
# Preprocessor Definitions
#==============================================================================

target_compile_definitions(LENSManager PRIVATE
    _WINDOWS
    STRICT
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

#==============================================================================
# Link Libraries
#==============================================================================

target_link_libraries(LENSManager PRIVATE
    comctl32
    ole32
    oleaut32
    uuid
    gdi32
    user32
    advapi32
    shell32
    htmlhelp
    uxtheme
    dwmapi
)

#==============================================================================
# Target Properties
#==============================================================================

set_target_properties(LENSManager PROPERTIES
    WIN32_EXECUTABLE TRUE
)

#==============================================================================
# Linker Options (manifest embedding)
#==============================================================================

if(MSVC)
    target_link_options(LENSManager PRIVATE
        /MANIFEST:EMBED
        "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/LENSManager.manifest"
    )
endif()

#==============================================================================
# Installation
#==============================================================================

install(TARGETS LENSManager
    RUNTIME DESTINATION bin
)
