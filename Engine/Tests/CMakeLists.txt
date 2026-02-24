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
    # Sprint 150-174: v8.3.0 Block test suites
    Sprint150_PluginTestMatrix.cpp
    Sprint151_PluginSandboxPolicy.cpp
    Sprint152_PluginCompatKitV2.cpp
    Sprint153_PluginReferencePack.cpp
    Sprint154_PluginTrustChain.cpp
    Sprint155_ARM64BuildConfig.cpp
    Sprint156_ARM64LibraryMatrix.cpp
    Sprint157_ARM64RuntimeValidator.cpp
    Sprint158_ARM64PerfBaseline.cpp
    Sprint159_ARM64CIIntegration.cpp
    Sprint160_JPEG2000Decoder.cpp
    Sprint161_CADFormat.cpp
    Sprint162_GLTFModel.cpp
    Sprint163_Scientific.cpp
    Sprint164_FallbackEngine.cpp
    Sprint165_ArchiveMemory.cpp
    Sprint166_ZeroCopy.cpp
    Sprint167_AdaptiveCache.cpp
    Sprint168_HotMode.cpp
    Sprint169_MemPressure.cpp
    Sprint170_MatrixValidation.cpp
    Sprint171_InstallerLifecycle.cpp
    Sprint172_ReleaseGate.cpp
    Sprint173_DocSync.cpp
    Sprint174_ProgramClosure.cpp
)

# Link JXL libraries if enabled
if(HAS_LIBJXL)
    target_compile_definitions(EngineTests PRIVATE JXL_STATIC_DEFINE)
endif()

# Benchmark executable
add_executable(EngineBenchmark
    EngineBenchmark.cpp
)

# Sprint 16: Integration Tests
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


