# ExplorerLens Validator Tool
# Sprint 13: Real-File Test Fixtures & Compatibility Kit

cmake_minimum_required(VERSION 3.26)

project(ExplorerLensValidator VERSION 7.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(ExplorerLensValidator
    ExplorerLensValidator.cpp
)

# Set output directory
set_target_properties(ExplorerLensValidator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "ExplorerLensValidator"
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(ExplorerLensValidator PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    
    # Link required Windows libraries
    target_link_libraries(ExplorerLensValidator PRIVATE
        psapi.lib
    )
    
    # Set subsystem to console
    set_target_properties(ExplorerLensValidator PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Installation
install(TARGETS ExplorerLensValidator
    RUNTIME DESTINATION bin
    COMPONENT tools
)

