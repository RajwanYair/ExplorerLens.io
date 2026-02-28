# ExplorerLens PluginHost Executable
# Separate process for secure plugin execution

cmake_minimum_required(VERSION 3.20)

# PluginHost executable
add_executable(PluginHost
    main.cpp
    PluginHostServer.cpp
    PluginHostServer.h
)

# Include directories
target_include_directories(PluginHost PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/SDK
)

# Link libraries
target_link_libraries(PluginHost PRIVATE
    ExplorerLensEngine
)

# Compiler settings
if(MSVC)
    target_compile_options(PluginHost PRIVATE
        /W4           # Warning level 4
        /WX-          # Warnings NOT as errors (matches project convention)
        /permissive-  # Standards conformance
        /Zc:__cplusplus  # Correct __cplusplus macro
    )

    # Link options
    target_link_options(PluginHost PRIVATE
        /SUBSYSTEM:CONSOLE     # Console application
        /DYNAMICBASE           # ASLR
        /NXCOMPAT              # DEP
        /HIGHENTROPYVA         # High entropy ASLR
    )
endif()

# Install
install(TARGETS PluginHost
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Post-build: Copy to output directory
add_custom_command(TARGET PluginHost POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:PluginHost>
        ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/PluginHost.exe
    COMMENT "Copying PluginHost.exe to output directory"
)
