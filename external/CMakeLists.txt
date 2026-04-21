# ExplorerLens External Libraries - Optimized Build Configuration
# VS2026 | x64 | Maximum Performance

cmake_minimum_required(VERSION 3.20)
project(ExplorerLensExternalLibs VERSION 4.6.0 LANGUAGES C CXX)

# VS2026 Optimization Flags
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /GL /Gy /GF /MD /guard:cf /arch:AVX2 /fp:fast /Qpar /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /GL /Gy /GF /MD /guard:cf /arch:AVX2 /fp:fast /Qpar /DNDEBUG /std:c++17")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF /DYNAMICBASE /NXCOMPAT")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")

# Build all libraries as static for optimal linking
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# zlib 1.3.1 - Modern compression library
message(STATUS "Configuring zlib 1.3.1...")
add_subdirectory(zlib-1.3.1/zlib-1.3.1)

# minizip-ng 4.0.10 - Modern ZIP library with ZIP64, AES, Unicode support
message(STATUS "Configuring minizip-ng 4.0.10...")
set(MZ_COMPAT OFF CACHE BOOL "Disable legacy compatibility mode" FORCE)
set(MZ_ZLIB ON CACHE BOOL "Enable zlib compression" FORCE)
set(MZ_BZIP2 OFF CACHE BOOL "Disable bzip2" FORCE)
set(MZ_LZMA OFF CACHE BOOL "Disable LZMA" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "Disable Zstandard" FORCE)
set(MZ_LIBCOMP OFF CACHE BOOL "Disable libcompression" FORCE)
set(MZ_FETCH_LIBS OFF CACHE BOOL "Don't fetch external libs" FORCE)
set(MZ_PKCRYPT ON CACHE BOOL "Enable PKWARE traditional encryption" FORCE)
set(MZ_WZAES ON CACHE BOOL "Enable WinZip AES encryption" FORCE)
set(MZ_OPENSSL OFF CACHE BOOL "Use Windows Crypto API instead" FORCE)
set(MZ_SIGNING OFF CACHE BOOL "Disable signing support" FORCE)
add_subdirectory(minizip-ng-4.0.10)

message(STATUS "")
message(STATUS "================================")
message(STATUS "ExplorerLens External Libraries")
message(STATUS "================================")
message(STATUS "zlib:       1.3.1 (with SIMD)")
message(STATUS "minizip-ng: 4.0.10 (ZIP64 + AES)")
message(STATUS "Optimizations: AVX2, Link-Time, /O2")
message(STATUS "================================")
message(STATUS "")
