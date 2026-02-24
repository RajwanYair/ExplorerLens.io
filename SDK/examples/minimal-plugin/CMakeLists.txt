# ExplorerLens Minimal Plugin Example - CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MinimalPlugin VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include SDK
set(EXPLORERLENS_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "ExplorerLens SDK directory")
include_directories(${EXPLORERLENS_SDK_DIR})

# Build plugin as shared library
add_library(minimal_plugin SHARED
    minimal_plugin.cpp
)

# Set output name
set_target_properties(minimal_plugin PROPERTIES
    OUTPUT_NAME "minimal_plugin"
    PREFIX ""  # No 'lib' prefix
)

# Export plugin functions
target_compile_definitions(minimal_plugin PRIVATE
    EXPLORERLENS_PLUGIN_EXPORTS
)

# Windows-specific settings
if(WIN32)
    # /MT runtime (static CRT)
    set_property(TARGET minimal_plugin PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Add .def file to export functions without name mangling
    target_sources(minimal_plugin PRIVATE minimal_plugin.def)
    
    # Set subsystem
    target_link_options(minimal_plugin PRIVATE
        /SUBSYSTEM:WINDOWS
    )
endif()

# Installation
install(TARGETS minimal_plugin
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Create plugin package
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt
    DESTINATION "plugins/minimal_plugin.dtplugin"
)

install(TARGETS minimal_plugin
    DESTINATION "plugins/minimal_plugin.dtplugin"
    RENAME "plugin.dll"
)


