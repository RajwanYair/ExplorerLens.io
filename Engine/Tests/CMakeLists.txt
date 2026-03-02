#==============================================================================
# ExplorerLens Engine Tests - CMake Configuration
#==============================================================================

cmake_minimum_required(VERSION 3.20)

project(EngineTests LANGUAGES CXX)

#==============================================================================
# Test Executable
#==============================================================================

add_executable(EngineTests
    EngineTests.cpp
    test_gpu.cpp
    PluginTestMatrixTests.cpp
    PluginSandboxPolicyTests.cpp
    PluginCompatKitTests.cpp
    PluginReferencePackTests.cpp
    PluginTrustChainTests.cpp
    ARM64BuildConfigTests.cpp
    ARM64LibraryMatrixTests.cpp
    ARM64RuntimeValidatorTests.cpp
    ARM64PerfBaselineTests.cpp
    ARM64CIIntegrationTests.cpp
    JPEG2000DecoderTests.cpp
    CADFormatDecoderTests.cpp
    GLTFModelDecoderTests.cpp
    ScientificDecoderTests.cpp
    FallbackEngineTests.cpp
    ArchiveMemoryTests.cpp
    ZeroCopyPipelineTests.cpp
    AdaptiveCacheBudgetTests.cpp
    HotModeDirectoryTests.cpp
    MemoryPressureTests.cpp
    MatrixValidationTests.cpp
    InstallerLifecycleTests.cpp
    ReleaseGateTests.cpp
    DocumentationSyncTests.cpp
    ProgramClosureTests.cpp
)

# Windows-specific defines (NOMINMAX prevents min/max macro conflicts)
if(WIN32)
    target_compile_definitions(EngineTests PRIVATE
        NOMINMAX
    )
endif()

# Link JXL libraries if enabled
if(HAS_LIBJXL)
    target_compile_definitions(EngineTests PRIVATE JXL_STATIC_DEFINE)
endif()

# Benchmark executable
add_executable(EngineBenchmark
    EngineBenchmark.cpp
)
add_executable(IntegrationTests
    IntegrationTests.cpp
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
        ExplorerLensEngine
        d3d12.lib
        dxgi.lib
)

# Link IntegrationTests
target_link_libraries(IntegrationTests
    PRIVATE
        ExplorerLensEngine
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

target_include_directories(IntegrationTests
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
        ExplorerLensEngine
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
    ${CMAKE_CURRENT_SOURCE_DIR}
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
        /wd4189     # C4189: local variable initialized but not referenced
                    # Expected in tests: assert() is no-op in Release, so
                    # variables used only in EXPECT_* macros appear unused
    )
    target_compile_options(EngineBenchmark PRIVATE
        /W4
        /wd4189     # Same rationale as above
    )

    # libwebp rebuilt with /MD — no NODEFAULTLIB:LIBCMT workaround needed
    target_link_options(EngineTests PRIVATE
        /IGNORE:4099            # Ignore missing PDB warnings
    )
    target_link_options(EngineBenchmark PRIVATE
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

if(BUILD_BENCHMARKS)
    add_test(
        NAME EngineBenchmarks
        COMMAND EngineBenchmark
    )
endif()

if(WIN32)
    # Build a proper Windows PATH string (semicolon-delimited, no CMake list artifacts)
    set(EXPLORERLENS_TEST_RUNTIME_DIRS
        "${CMAKE_SOURCE_DIR}/external/image-libs/libde265-1.0.15/build-vs/libde265/Release"
        "${CMAKE_SOURCE_DIR}/external/image-libs/libheif-1.19.5/build-vs/libheif/Release"
        "${CMAKE_SOURCE_DIR}/external/image-libs/libheif-1.19.5/install/bin"
        "${CMAKE_SOURCE_DIR}/external/compression-libs/unrar/build/unrardll64/Release"
        "${CMAKE_SOURCE_DIR}/external/camera-libs/libraw/buildfiles/release-x86_64"
        "${CMAKE_SOURCE_DIR}/external/compression-libs/zlib-1.3.1/build-md"
        "${CMAKE_SOURCE_DIR}/external/compression-libs/zstd-1.5.7/build/cmake/build-md/lib"
    )
    # Join list elements with ";" as Windows PATH separator
    string(JOIN "\\;" EXPLORERLENS_TEST_RUNTIME_PATH ${EXPLORERLENS_TEST_RUNTIME_DIRS})

    set_tests_properties(EngineUnitTests PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit"
        TIMEOUT 30
    )

    if(BUILD_BENCHMARKS)
        set_tests_properties(EngineBenchmarks PROPERTIES
            ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
            LABELS "benchmark"
            TIMEOUT 120
        )
    endif()
endif()
