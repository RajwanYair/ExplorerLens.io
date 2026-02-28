# CMake Toolchain File — Windows ARM64 (Cross-compile from x64 host)
# Usage: cmake -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-windows-arm64.cmake

cmake_minimum_required(VERSION 3.20)

#==============================================================================
# Target system
#==============================================================================

set(CMAKE_SYSTEM_NAME       Windows)
set(CMAKE_SYSTEM_PROCESSOR  ARM64)

#==============================================================================
# Compiler identification
# When using the Visual Studio / MSVC generator with ilammy/msvc-dev-cmd
# (amd64_arm64 arch), the cross-compiler is already on PATH. We just need to
# declare the target architecture so CMake passes /arm64 to the linker and
# selects the correct runtime libraries.
#==============================================================================

# Force MSVC to target ARM64
set(CMAKE_C_COMPILER   cl)
set(CMAKE_CXX_COMPILER cl)

# Compiler flags for ARM64 target
set(CMAKE_C_FLAGS_INIT   "/DARM64 /D_ARM64_ /DWIN64 /D_WIN64")
set(CMAKE_CXX_FLAGS_INIT "/DARM64 /D_ARM64_ /DWIN64 /D_WIN64")

# Machine type for the linker
set(CMAKE_EXE_LINKER_FLAGS_INIT    "/MACHINE:ARM64")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "/MACHINE:ARM64")
set(CMAKE_STATIC_LINKER_FLAGS_INIT "")

#==============================================================================
# ARM64 build marker (used by Engine/Utils/ARM64BuildConfig.h)
#==============================================================================

set(ARM64_BUILD ON CACHE BOOL "Building for Windows ARM64 target" FORCE)

#==============================================================================
# Disable x64-specific intrinsics / SSE options
#==============================================================================

# Ensure no SSE2 flags are injected (they are the x64 default in MSVC)
string(REPLACE "/arch:SSE2" "" CMAKE_C_FLAGS_INIT   "${CMAKE_C_FLAGS_INIT}")
string(REPLACE "/arch:SSE2" "" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")

#==============================================================================
# Output directories (keep separate from x64 artifacts)
#==============================================================================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm64/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm64/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm64/bin")

#==============================================================================
# CTest / test cross-execution note
# Cross-compiled tests cannot run on an x64 host. Set RUN_TESTS to OFF
# when cross-compiling, or use a Windows-on-ARM device / emulator.
#==============================================================================

set(ExplorerLens_ARM64_CROSS_COMPILE ON)
set(CTEST_CUSTOM_PRE_TEST
    "message(STATUS \"[ARM64] Cross-compiled tests skipped on x64 host.\")")

#==============================================================================
# Find-package search paths
# Restrict find_library / find_path to ARM64 build outputs only so that
# x64 libraries from the host are never accidentally linked.
#==============================================================================

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # Use host programs (cmake, ninja…)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # Link only ARM64 libraries
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # Include only target headers
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#==============================================================================
# Status message
#==============================================================================

message(STATUS "[ExplorerLens] Toolchain: Windows ARM64 cross-compile (host: amd64)")
message(STATUS "[ExplorerLens] MSVC cross-compiler: cl (amd64_arm64)")
message(STATUS "[ExplorerLens] Linker machine: ARM64")

