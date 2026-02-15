#==============================================================================
# DarkThumbs Engine Tests - CMake Configuration
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(EngineTests LANGUAGES CXX)

#==============================================================================
# Test Executable
#==============================================================================

add_executable(EngineTests
    EngineTests.cpp
    test_gpu.cpp
)

# Link JXL libraries if enabled
if(HAS_LIBJXL)
    target_compile_definitions(EngineTests PRIVATE JXL_STATIC_DEFINE)
endif()

# Benchmark executable
add_executable(EngineBenchmark
    EngineBenchmark.cpp
)

# Link JXL libraries if enabled
if(HAS_LIBJXL)
    target_compile_definitions(EngineBenchmark PRIVATE JXL_STATIC_DEFINE)
endif()

# Add library search paths
target_link_directories(EngineBenchmark
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/libjxl-install/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/brotli-install/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/highway-install/lib
)

target_link_libraries(EngineBenchmark
    PRIVATE
        DarkThumbsEngine
)

# Link JXL libraries if enabled
if(HAS_LIBJXL)
    target_link_libraries(EngineBenchmark
        PRIVATE
            jxl
            jxl_threads
            hwy
            brotlicommon
            brotlidec
    )
endif()

target_include_directories(EngineBenchmark
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Add library search paths
target_link_directories(EngineTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/libjxl-install/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/brotli-install/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../../external/highway-install/lib
)

target_link_libraries(EngineTests
    PRIVATE
        DarkThumbsEngine
)

# Link JXL libraries if enabled
if(HAS_LIBJXL)
    target_link_libraries(EngineTests
        PRIVATE
            jxl
            jxl_threads
            hwy
            brotlicommon
            brotlidec
    )
endif()

target_include_directories(EngineTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Windows-specific settings
if(WIN32)
    target_link_libraries(EngineTests
        PRIVATE
            shlwapi
    )
endif()

# Compiler warnings and linker options
if(MSVC)
    target_compile_options(EngineTests PRIVATE
        /W4
    )
    target_compile_options(EngineBenchmark PRIVATE
        /W4
    )
    
    # Fix LIBCMT conflict - external libraries built with /MT need this
    target_link_options(EngineTests PRIVATE
        /NODEFAULTLIB:LIBCMT    # Ignore static CRT conflicts from external libs
        /IGNORE:4099            # Ignore missing PDB warnings
    )
    target_link_options(EngineBenchmark PRIVATE
        /NODEFAULTLIB:LIBCMT    # Ignore static CRT conflicts from external libs
        /IGNORE:4099            # Ignore missing PDB warnings
    )
endif()

#==============================================================================
# Register Tests
#==============================================================================

add_test(
    NAME EngineUnitTests
    COMMAND EngineTests
)
