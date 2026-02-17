# DarkThumbs Validator Tool
# Sprint 13: Real-File Test Fixtures & Compatibility Kit

cmake_minimum_required(VERSION 3.26)

project(DarkThumbsValidator VERSION 7.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(DarkThumbsValidator
    DarkThumbsValidator.cpp
)

# Set output directory
set_target_properties(DarkThumbsValidator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "DarkThumbsValidator"
)

# Windows-specific settings
if(WIN32)
    target_compile_definitions(DarkThumbsValidator PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    
    # Link required Windows libraries
    target_link_libraries(DarkThumbsValidator PRIVATE
        psapi.lib
    )
    
    # Set subsystem to console
    set_target_properties(DarkThumbsValidator PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Installation
install(TARGETS DarkThumbsValidator
    RUNTIME DESTINATION bin
    COMPONENT tools
)
