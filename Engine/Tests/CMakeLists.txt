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
    EngineTests_Core.cpp
    EngineTests_Features.cpp
    EngineTests_Mid.cpp
    EngineTests_Late.cpp
    EngineTests_Platform.cpp
    test_gpu.cpp
    PluginCompatKitTests.cpp
    PluginReferencePackTests.cpp
    JPEG2000DecoderTests.cpp
    CADFormatDecoderTests.cpp
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
    # CLI command implementations (linked directly for CLI unit tests in EngineTests_Late.cpp)
    ${CMAKE_SOURCE_DIR}/src/Tools.CLI/BenchmarkCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/Tools.CLI/CacheCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/Tools.CLI/CommandRouter.cpp
    ${CMAKE_SOURCE_DIR}/src/Tools.CLI/DoctorCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/Tools.CLI/InfoCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/Tools.CLI/RegisterCommand.cpp
)

# Windows-specific defines (NOMINMAX prevents min/max macro conflicts)
if(WIN32)
    target_compile_definitions(EngineTests PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _WIN32_WINNT=0x0A00
        NTDDI_VERSION=0x0A000006   # Windows 10 1909 — required for ID2D1DeviceContext5
        WINVER=0x0A00
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
        /bigobj                     # C1128: split test files (5 .cpp files totalling ~50K lines) may exceed
                                    # COFF section limit without /bigobj
    )
    target_compile_options(EngineBenchmark PRIVATE
        /W4
    )

    # libwebp rebuilt with /MD — suppress LIBCMT conflict from static runtime libs
    target_link_options(EngineTests PRIVATE
        /IGNORE:4099            # Ignore missing PDB warnings
        /NODEFAULTLIB:LIBCMT    # Suppress LNK4098: LIBCMT conflict with /MD CRT
    )
    target_link_options(IntegrationTests PRIVATE
        /IGNORE:4099            # Ignore missing PDB warnings
        /NODEFAULTLIB:LIBCMT    # Suppress LIBCMT conflict with /MD CRT
    )
    target_link_options(EngineBenchmark PRIVATE
        /IGNORE:4099            # Ignore missing PDB warnings
        /NODEFAULTLIB:LIBCMT    # Suppress LIBCMT conflict with /MD CRT
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

    set_tests_properties(SubsystemTest.Integration PROPERTIES
        ENVIRONMENT "PATH=${EXPLORERLENS_TEST_RUNTIME_PATH}\\;$ENV{PATH}"
        LABELS "integration;subsystem"
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

# =============================================================================
# Catch2 Tests (Sprint 46 — DecoderUnit + FormatDetection + CacheSubsystem)
# =============================================================================
# Enable with: cmake -DBUILD_CATCH2_TESTS=ON (default ON per ROADMAP R2)
# The Catch2 executable is named EngineCatch2Tests and runs via ctest -L catch2

option(BUILD_CATCH2_TESTS "Build Catch2-based unit tests (requires Catch2 v3)" ON)

if(BUILD_CATCH2_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(EngineCatch2Tests
        Catch2Tests/FormatDetectionTests.cpp
        Catch2Tests/CacheSubsystemTests.cpp
        Catch2Tests/DecoderUnitTests.cpp
        Catch2Tests/MagicBytesDatabaseTests.cpp
        Catch2Tests/PerceptualHashTests.cpp
        Catch2Tests/StreamingDecoderTests.cpp
        Catch2Tests/FormatStatusTests.cpp
        Catch2Tests/CorpusValidationTests.cpp
        Catch2Tests/InputValidationTests.cpp
        Catch2Tests/DecoderRegistryTests.cpp
        Catch2Tests/SafeDimensionsTests.cpp
        Catch2Tests/PipelineIntegrationTests.cpp
        Catch2Tests/MetadataExtractionTests.cpp
        Catch2Tests/ResultTypeTests.cpp
        Catch2Tests/ProbeHeaderTests.cpp
        Catch2Tests/DecoderFallbackChainTests.cpp
        Catch2Tests/CacheKeyTests.cpp
        Catch2Tests/CLICommandParserTests.cpp
        Catch2Tests/ColorSpaceTests.cpp
    )

    target_include_directories(EngineCatch2Tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_link_libraries(EngineCatch2Tests
        PRIVATE
            ExplorerLensEngine
            Catch2::Catch2WithMain
    )

    if(WIN32)
        target_compile_definitions(EngineCatch2Tests PRIVATE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _WIN32_WINNT=0x0A00
        )
        target_link_libraries(EngineCatch2Tests PRIVATE shlwapi)
    endif()

    # Pass corpus root path so CorpusValidationTests can find test files
    target_compile_definitions(EngineCatch2Tests PRIVATE
        CORPUS_ROOT="${CMAKE_SOURCE_DIR}/data/corpus"
    )

    if(MSVC)
        target_compile_options(EngineCatch2Tests PRIVATE /W4 /bigobj)
        target_link_options(EngineCatch2Tests PRIVATE
            /IGNORE:4099
            /NODEFAULTLIB:LIBCMT
        )
    endif()

    include(CTest)
    include(Catch)
    catch_discover_tests(EngineCatch2Tests
        TEST_PREFIX "Catch2."
        REPORTER junit
        OUTPUT_DIR  "${CMAKE_BINARY_DIR}"
        OUTPUT_FILE catch2-results.xml
        PROPERTIES  LABELS "catch2;unit"
    )
endif() # BUILD_CATCH2_TESTS
