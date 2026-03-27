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
    # Sprint 25 + 29: Integration Test Framework + COM Integration (also in IntegrationTests)
    Integration/IntegrationTestRunner.cpp
    Integration/COMIntegrationTest.cpp
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

# Propagate LibRaw availability so RAW decoder tests can skip gracefully
if(HAS_LIBRAW)
    target_compile_definitions(EngineTests PRIVATE HAS_LIBRAW)
endif()

# Benchmark executable
add_executable(EngineBenchmark
    EngineBenchmark.cpp
)
add_executable(IntegrationTests
    IntegrationTests.cpp
    # Sprint 25: Integration Test Framework
    Integration/IntegrationTestRunner.cpp
    # Sprint 29: COM Integration Test
    Integration/COMIntegrationTest.cpp
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
target_include_directories(IntegrationTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/Integration
)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/Integration
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
        $<$<CONFIG:Debug>:/bigobj>  # C1128: EngineTests.cpp (22K+ lines) exceeds
                                    # COFF section limit in Debug without /bigobj
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

# ---------------------------------------------------------------------------
# Per-subsystem CTest label scaffold (PERF-01)
#
# All subsystem tests run through the monolithic EngineTests executable.
# Individual targets are registered here with distinct LABELS so they can be
# filtered independently:
#   ctest -L cache          → cache + adaptive-budget tests only
#   ctest -L gpu            → GPU decode & vendor-routing tests only
#   ctest -L decoder        → format-decoder tests only
#   ctest -L plugin         → plugin ecosystem tests only
#   ctest -L memory         → memory-pressure & compactor tests only
#   ctest -L pipeline       → zero-copy & parallel-I/O tests only
#   ctest -L arm64          → ARM64 build/runtime tests only
#   ctest -L utils          → installer, matrix, release-gate tests only
#   ctest -L integration    → cross-subsystem integration tests only
#
# When EngineTests.cpp is eventually split into per-subsystem executables
# (Sprint 25+), replace COMMAND EngineTests with the subsystem-specific exe.
# ---------------------------------------------------------------------------

add_test(NAME SubsystemTest.Cache    COMMAND EngineTests)
add_test(NAME SubsystemTest.GPU      COMMAND EngineTests)
add_test(NAME SubsystemTest.Decoder  COMMAND EngineTests)
add_test(NAME SubsystemTest.Plugin   COMMAND EngineTests)
add_test(NAME SubsystemTest.Memory   COMMAND EngineTests)
add_test(NAME SubsystemTest.Pipeline COMMAND EngineTests)
add_test(NAME SubsystemTest.Utils    COMMAND EngineTests)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTests.cpp")
    add_test(NAME SubsystemTest.Integration COMMAND IntegrationTests)
endif()

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
        LABELS "unit;all"
        TIMEOUT 300
    )

    set_tests_properties(SubsystemTest.Cache PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;cache;subsystem"
        TIMEOUT 60
    )
    set_tests_properties(SubsystemTest.GPU PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;gpu;subsystem"
        TIMEOUT 60
    )
    set_tests_properties(SubsystemTest.Decoder PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;decoder;subsystem"
        TIMEOUT 60
    )
    set_tests_properties(SubsystemTest.Plugin PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;plugin;subsystem"
        TIMEOUT 60
    )
    set_tests_properties(SubsystemTest.Memory PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;memory;subsystem"
        TIMEOUT 60
    )
    set_tests_properties(SubsystemTest.Pipeline PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;pipeline;subsystem"
        TIMEOUT 60
    )
    set_tests_properties(SubsystemTest.Utils PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "unit;utils;subsystem"
        TIMEOUT 60
    )

    if(BUILD_BENCHMARKS)
        set_tests_properties(EngineBenchmarks PROPERTIES
            ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
            LABELS "benchmark"
            TIMEOUT 120
        )
    endif()
endif()
